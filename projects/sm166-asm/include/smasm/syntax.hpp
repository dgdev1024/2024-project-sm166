/** @file smasm/syntax.hpp */

#pragma once

#include <smasm/keyword.hpp>

namespace smasm
{

  /** Syntax Type Enumeration *********************************************************************/

  enum class syntax_type
  {
    program,

    size_directive,
    section_directive,

    label_statement,
    data_statement,
    include_statement,
    instruction_statement,

    address_literal,
    binary_expression,

    identifier,
    numeric_literal,
    string_literal
  };

  /** Syntax Statement Base Class *****************************************************************/

  class statement
  {
  public:
    using ptr       = std::shared_ptr<statement>;
    using body      = std::vector<ptr>;

  protected:
    inline statement (
      const syntax_type type
    ) :
      m_syntax_type { type }
    {}

  public:
    virtual void dump (std::ostream& os, std::size_t i = 0) const = 0;

  public:
    template <typename T, typename... As>
    inline static std::shared_ptr<T> make (As&&... args)
    {
      static_assert(std::is_base_of_v<statement, T>, "'T' must derive from 'smasm::statement'.");
      return std::make_shared<T>(std::forward<As>(args)...);
    }

  public:
    inline syntax_type get_syntax_type () const { return m_syntax_type; }

  protected:
    syntax_type m_syntax_type;

  };

  /** Syntax Expression Base Class ****************************************************************/

  class expression : public statement
  {
  public:
    using ptr = std::shared_ptr<expression>;
    using array = std::vector<ptr>;

  protected:
    inline expression (
      const syntax_type type
    ) :
      statement { type }
    {}

  public:
    template <typename T, typename... As>
    inline static std::shared_ptr<T> make (As&&... args)
    {
      static_assert(std::is_base_of_v<expression, T>, "'T' must derive from 'smasm::expression'.");
      return std::make_shared<T>(std::forward<As>(args)...);
    }

  };

  /** Syntax Casting Functions ********************************************************************/

  template <typename T>
  inline std::shared_ptr<T> statement_cast (const statement::ptr& ptr)
  {
    static_assert(std::is_base_of_v<statement, T>, "'T' must derive from 'smasm::statement'.");
    return std::static_pointer_cast<T>(ptr);
  }

  template <typename T>
  inline std::shared_ptr<T> expression_cast (const expression::ptr& ptr)
  {
    static_assert(std::is_base_of_v<expression, T>, "'T' must derive from 'smasm::expression'.");
    return std::static_pointer_cast<T>(ptr);
  }

  /** Program Statement ***************************************************************************/

  class program : public statement
  {
  public:
    using ptr = std::shared_ptr<program>;

  public:
    inline program () :
      statement { syntax_type::program }
    {}

  public:
    inline virtual void dump (std::ostream& os, std::size_t i = 0) const override
    {
      os << indent(i) << "program {\n";
      {
        for (const auto& stmt : m_body) {
          stmt->dump(os, i + 2);
        }
      }
      os << indent(i) << "}\n";
    }

    inline void push (const statement::ptr& stmt)
    {
      m_body.push_back(stmt);
    }

  public:
    inline const statement::body& get_body () const { return m_body; }

  private:
    statement::body m_body;

  };

  /** Directives **********************************************************************************/

  class size_directive : public statement
  {
  public:
    inline size_directive (
      const std::uint64_t size
    ) :
      statement { syntax_type::size_directive },
      m_size    { size }
    {}

  public:
    inline virtual void dump (std::ostream& os, std::size_t i = 0) const override
    {
      os << indent(i) << ".size directive: " << m_size << "\n";
    }

  public:
    inline std::uint64_t get_size () const { return m_size; }

  private:
    std::uint64_t m_size = 0;

  };

  class section_directive : public statement
  {
  public:
    inline section_directive (
      bool is_ram,
      const expression::ptr& address_expr
    ) :
      statement       { syntax_type::section_directive },
      m_is_ram        { is_ram },
      m_address_expr  { address_expr }
    {}

  public:
    inline virtual void dump (std::ostream& os, std::size_t i = 0) const override
    {
      os << indent(i) << ".section directive {\n";
      {
        os << indent(i + 2) << "section: " << (m_is_ram == true ? "ram" : "rom") << "\n";
        if (m_address_expr != nullptr) {
          os << indent(i + 2) << "address expression:\n";
          m_address_expr->dump(os, i + 4);
        }
      }
      os << indent(i) << "}\n";
    }

  public:
    inline bool is_ram () const { return m_is_ram; }
    inline const expression::ptr& get_address_expr () const { return m_address_expr; }

  private:
    bool m_is_ram = false;
    expression::ptr m_address_expr = nullptr;

  };

  /** Statements **********************************************************************************/

  class label_statement : public statement
  {
  public:
    label_statement (
      const expression::ptr& label
    ) :
      statement { syntax_type::label_statement },
      m_label   { label }
    {}

  public:
    inline virtual void dump (std::ostream& os, std::size_t i = 0) const override
    {
      os << indent(i) << "label: {\n";
      if (m_label != nullptr) {
        m_label->dump(os, i + 2);
      }
      os << indent(i) << "}\n";
    }

  public:
    inline const expression::ptr& get_label () const { return m_label; }

  private:
    expression::ptr m_label = nullptr;

  };

  class data_statement : public statement
  {
  public:
    data_statement (
      const expression::array& array,
      int size = 1
    ) :
      statement { syntax_type::data_statement },
      m_array { array },
      m_size { size }
    {}

  public:
    inline virtual void dump (std::ostream& os, std::size_t i = 0) const override
    {
      os << indent(i) << "data statement";

      switch (m_size) {
        case 1: os << ": byte"; break;
        case 2: os << ": word"; break;
        case 4: os << ": long"; break;
      }
      
      os << " {\n";
      for (const auto& expr : m_array) {
        expr->dump(os, i + 2);
      }
      os << indent(i) << "}\n";
    }

  public:
    inline const expression::array& get_array () const { return m_array; }
    inline int get_size () const { return m_size; }
    inline bool is_byte () const { return m_size == 1; }
    inline bool is_word () const { return m_size == 2; }
    inline bool is_long () const { return m_size == 4; }

  private:
    expression::array m_array;
    int m_size = 0;

  };

  class instruction_statement : public statement
  {
  public:
    inline instruction_statement (
      const std::string& mnemonic,
      const expression::ptr& first,
      const expression::ptr& second
    ) :
      statement   { syntax_type::instruction_statement },
      m_mnemonic  { mnemonic },
      m_first     { first },
      m_second    { second }
    {}

  public:
    inline virtual void dump (std::ostream& os, std::size_t i = 0) const override
    {
      os << indent(i) << "instruction {\n";
      {
        os << indent(i + 2) << "mnemonic: \"" << m_mnemonic << "\"\n";

        if (m_first != nullptr) {
          os << indent(i + 2) << "first:\n";
          m_first->dump(os, i + 4);
        }

        if (m_second != nullptr) {
          os << indent(i + 2) << "second:\n";
          m_second->dump(os, i + 4);
        }
      }
      os << indent(i) << "}\n";
    }

  public:
    inline const std::string& get_mnemonic () const { return m_mnemonic; }
    inline const expression::ptr& get_first () const { return m_first; }
    inline const expression::ptr& get_second () const { return m_second; }

  private:
    std::string     m_mnemonic  = "";
    expression::ptr m_first     = nullptr;
    expression::ptr m_second    = nullptr;

  };

  class include_statement : public statement
  {
  public:
    include_statement (
      const expression::ptr& filename_expr
    ) :
      statement { syntax_type::include_statement },
      m_filename_expr { filename_expr }
    {}

  public:
    inline virtual void dump (std::ostream& os, std::size_t i = 0) const override
    {
      os << indent(i) << "include {\n";
      if (m_filename_expr != nullptr) {
        m_filename_expr->dump(os, i + 2);
      }
      os << indent(i) << "}\n";
    }

  public:
    inline const expression::ptr& get_filename_expr () const { return m_filename_expr; }

  private:
    expression::ptr m_filename_expr = nullptr;

  };

  /** Expressions *********************************************************************************/

  class address_literal : public expression
  {
  public:
    inline address_literal (
      const expression::ptr& address_expr
    ) :
      expression      { syntax_type::address_literal },
      m_address_expr  { address_expr }
    {}

  public:
    inline virtual void dump (std::ostream& os, std::size_t i = 0) const override
    {
      os << indent(i) << "address literal {\n";
      {
        if (m_address_expr != nullptr) {
          os << indent(i + 2) << "address:\n";
          m_address_expr->dump(os, i + 4);
        }
      }
      os << indent(i) << "}\n";
    }

  public:
    inline const expression::ptr& get_address_expr () const { return m_address_expr; }

  private:
    expression::ptr m_address_expr = nullptr;

  };

  class binary_expression : public expression
  {
  public:
    inline binary_expression (
      const expression::ptr& left,
      const expression::ptr& right,
      const std::string& oper
    ) :
      expression  { syntax_type::binary_expression },
      m_left      { left },
      m_right     { right },
      m_oper      { oper }
    {}

  public:
    inline virtual void dump (std::ostream& os, std::size_t i = 0) const override
    {
      os << indent(i) << "binary expression {\n";
      {
        if (m_left != nullptr) {
          os << indent(i + 2) << "left:\n";
          m_left->dump(os, i + 4);
        }

        os << indent(i + 2) << "operator: \"" << m_oper << "\"\n";

        if (m_right != nullptr) {
          os << indent(i + 2) << "right:\n";
          m_right->dump(os, i + 4);
        }
      }
      os << indent(i) << "}\n";
    }

  public:
    inline const expression::ptr& get_left () const { return m_left; }
    inline const expression::ptr& get_right () const { return m_right; }
    inline const std::string& get_oper () const { return m_oper; }

  private:
    expression::ptr m_left  = nullptr;
    expression::ptr m_right = nullptr;
    std::string     m_oper  = "";

  };

  /** Primary Expressions *************************************************************************/

  class identifier : public expression
  {
  public:
    inline identifier (
      const std::string& symbol
    ) :
      expression { syntax_type::identifier },
      m_symbol { symbol }
    {}

  public:
    inline virtual void dump (std::ostream& os, std::size_t i = 0) const override
    {
      os << indent(i) << "identifier: " << m_symbol << "\n";
    }

  public:
    inline const std::string& get_symbol () const { return m_symbol; }
    inline const keyword& get_keyword () const { return keyword::lookup(m_symbol); }

  private:
    std::string m_symbol = "";

  };
  
  class numeric_literal : public expression
  {
  public:
    inline numeric_literal (
      const double value
    ) :
      expression { syntax_type::numeric_literal },
      m_value { value }
    {}
  
  public:
    inline virtual void dump (std::ostream& os, std::size_t i = 0) const override
    {
      os << indent(i) << "numeric literal: " << m_value << "\n";
    }
  
  public:
    inline double get_number () const { return m_value; }
    inline std::uint64_t get_integer () const { return static_cast<std::uint64_t>(m_value); }
  
  private:
    double m_value = 0.0;
      
  };
  
  class string_literal : public expression
  {
  public:
    inline string_literal (
      const std::string& value
    ) :
      expression { syntax_type::string_literal },
      m_value { value }
    {}
  
  public:
    inline virtual void dump (std::ostream& os, std::size_t i = 0) const override
    {
      os << indent(i) << "string literal: \"" << m_value << "\"\n";
    }
  
  public:
    inline const std::string& get_string () const { return m_value; }
  
  private:
    std::string m_value = "";
      
  };

}
