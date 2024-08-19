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

    variable_declaration_statement,
    label_statement,
    data_statement,
    include_statement,
    incbin_statement,
    repeat_statement,
    shift_statement,
    if_statement,
    charmap_statement,
    newcharmap_statement,
    setcharmap_statement,
    instruction_statement,

    address_literal,
    function_expression,
    unary_expression,
    binary_expression,
    call_expression,

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

  class variable_declaration_statement : public statement
  {
  public:
    inline variable_declaration_statement (
      const expression::ptr& key_expr,
      const expression::ptr& value_expr,
      bool constant = false,
      bool global = false
    ) :
      statement { syntax_type::variable_declaration_statement },
      m_key_expr { key_expr },
      m_value_expr { value_expr },
      m_constant { constant },
      m_global { global }
    {}

  public:
    inline virtual void dump (std::ostream& os, std::size_t i = 0) const override
    {
      os  << indent(i) << (m_global == true ? "global " : "local ") 
          << (m_constant == true ? "constant" : "variable") << " {\n";
      if (m_key_expr != nullptr) {
        os << indent(i + 2) << "key\n";
        m_key_expr->dump(os, i + 4);
      }
      if (m_value_expr != nullptr) {
        os << indent(i + 2) << "value\n";
        m_value_expr->dump(os, i + 4);
      }
      os << indent(i) << "}\n";
    }

  public:
    inline const expression::ptr& get_key_expr () const { return m_key_expr; }
    inline const expression::ptr& get_value_expr () const { return m_value_expr; }
    inline bool is_constant () const { return m_constant; }
    inline bool is_global () const { return m_global; }

  private:
    expression::ptr m_key_expr = nullptr;
    expression::ptr m_value_expr = nullptr;
    bool m_constant = false;
    bool m_global = false;

  };

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
      int size = 1,
      bool offset = false
    ) :
      statement { syntax_type::data_statement },
      m_array { array },
      m_size { size },
      m_offset { offset }
    {}

  public:
    inline virtual void dump (std::ostream& os, std::size_t i = 0) const override
    {
      os << indent(i) << ((m_offset == true) ? "offset" : "data") << " statement";

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
    inline bool is_offset () const { return m_offset; }

  private:
    expression::array m_array;
    int m_size = 0;
    bool m_offset = false;

  };
  
  class charmap_statement : public statement
  {
  public:
    inline charmap_statement (
      const expression::ptr& char_expr,
      const expression::ptr& map_expr
    ) :
      statement { syntax_type::charmap_statement },
      m_char_expr { char_expr },
      m_map_expr { map_expr }
    {}
  
  public:
    inline virtual void dump (std::ostream& os, std::size_t i = 0) const override
    {
      os << indent(i) << "charmap statement {\n";
      {
        if (m_char_expr != nullptr) {
          os << indent(i + 2) << "chars {\n";
          m_char_expr->dump(os, i + 4);
          os << indent(i + 2) << "}\n";
        }
        if (m_map_expr != nullptr) {
          os << indent(i + 2) << "mapping {\n";
          m_map_expr->dump(os, i + 4);
          os << indent(i + 2) << "}\n";
        }
      }
      os << indent(i) << "}\n";
    }
  
  public:
    inline const expression::ptr& get_char_expr () const { return m_char_expr; }
    inline const expression::ptr& get_map_expr () const { return m_map_expr; }
  
  private:
    expression::ptr m_char_expr = nullptr;
    expression::ptr m_map_expr = nullptr;
    
  };
  
  class newcharmap_statement : public statement
  {
  public:
    inline newcharmap_statement (
      const expression::ptr& name_expr
    ) :
      statement { syntax_type::newcharmap_statement },
      m_name_expr { name_expr }
    {}
  
  public:
    inline virtual void dump (std::ostream& os, std::size_t i = 0) const override
    {
      os << indent(i) << "newcharmap statement {\n";
      {
        if (m_name_expr != nullptr) {
          os << indent(i + 2) << "name {\n";
          m_name_expr->dump(os, i + 4);
          os << indent(i + 2) << "}\n";
        }
      }
      os << indent(i) << "}\n";
    }
    
  public:
    inline const expression::ptr& get_name_expr () const { return m_name_expr; }
  
  private:
    expression::ptr m_name_expr = nullptr;
    
  };
  
  class setcharmap_statement : public statement
  {
  public:
    inline setcharmap_statement (
      const expression::ptr& name_expr
    ) :
      statement { syntax_type::setcharmap_statement },
      m_name_expr { name_expr }
    {}
  
  public:
    inline virtual void dump (std::ostream& os, std::size_t i = 0) const override
    {
      os << indent(i) << "setcharmap statement {\n";
      {
        if (m_name_expr != nullptr) {
          os << indent(i + 2) << "name {\n";
          m_name_expr->dump(os, i + 4);
          os << indent(i + 2) << "}\n";
        }
      }
      os << indent(i) << "}\n";
    }
    
  public:
    inline const expression::ptr& get_name_expr () const { return m_name_expr; }
  
  private:
    expression::ptr m_name_expr = nullptr;
    
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
  
  class repeat_statement : public statement
  {
  public:
    inline repeat_statement (
      const expression::ptr& count_expr,
      const statement::body& body
    ) :
      statement { syntax_type::repeat_statement },
      m_count_expr { count_expr },
      m_body { body }
    {}
  
  public:
    inline virtual void dump (std::ostream& os, std::size_t i = 0) const override
    {
      os << indent(i) << "repeat statement {\n";
      {
        if (m_count_expr != nullptr)
        {
          os << indent(i + 2) << "count\n";
          m_count_expr->dump(os, i + 4);
        }
        
        if (m_body.empty() == false)
        {
          os << indent(i + 2) << "body {\n";
          for (const auto& stmt : m_body)
          {
            stmt->dump(os, i + 4);
          }
          os << indent(i + 2) << "}\n";
        }
      }
      os << indent(i) << "}\n";
    }
  
  public:
    inline const expression::ptr& get_count_expr () const { return m_count_expr; }
    inline const statement::body& get_body () const { return m_body; }
  
  private:
    expression::ptr m_count_expr = nullptr;
    statement::body m_body;
  
  };
  
  class shift_statement : public statement
  {
  public:
    inline shift_statement (
      const expression::ptr& count_expr
    ) :
      statement { syntax_type::shift_statement },
      m_count_expr { count_expr }
    {}
  
  public:
    inline virtual void dump (std::ostream& os, std::size_t i = 0) const override
    {
      os << indent(i) << "shift statement {\n";
      {
        if (m_count_expr != nullptr)
        {
          os << indent(i + 2) << "count\n";
          m_count_expr->dump(os, i + 4);
        }
      }
      os << indent(i) << "}\n";
    }
  
  public:
    inline const expression::ptr& get_count_expr () const { return m_count_expr; }
  
  private:
    expression::ptr m_count_expr = nullptr;
  
  };
  
  class if_statement : public statement
  {
  public:
    inline if_statement (
      const expression::ptr& clause_expr,
      const statement::body& then_body,
      const statement::body& else_body
    ) :
      statement { syntax_type::if_statement },
      m_clause_expr { clause_expr },
      m_then_body { then_body },
      m_else_body { else_body }
    {}
    
  public:
    inline virtual void dump (std::ostream& os, std::size_t i = 0) const override
    {
      os << indent(i) << "if statement {\n";
      {
        if (m_clause_expr != nullptr)
        {
          os << indent(i + 2) << "clause\n";
          m_clause_expr->dump(os, i + 4);
        }
        
        if (m_then_body.empty() == false)
        {
          os << indent(i + 2) << "then {\n";
          for (const auto& stmt : m_then_body)
          {
            stmt->dump(os, i + 4);
          }
          os << indent(i + 2) << "}\n";
        }
        
        if (m_else_body.empty() == false)
        {
          os << indent(i + 2) << "else {\n";
          for (const auto& stmt : m_else_body)
          {
            stmt->dump(os, i + 4);
          }
          os << indent(i + 2) << "}\n";
        }
      }
      os << indent(i) << "}\n";
    }
  
  public:
    inline const expression::ptr& get_clause_expr () const { return m_clause_expr; }
    inline const statement::body& get_then_body () const { return m_then_body; }
    inline const statement::body& get_else_body () const { return m_else_body; }
  
  private:
    expression::ptr m_clause_expr = nullptr;
    statement::body m_then_body;
    statement::body m_else_body;
    
  };

  class include_statement : public statement
  {
  public:
    inline include_statement (
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

  class incbin_statement : public statement
  {
  public:
    inline incbin_statement (
      const expression::ptr& filename_expr
    ) :
      statement { syntax_type::incbin_statement },
      m_filename_expr { filename_expr }
    {}

  public:
    inline virtual void dump (std::ostream& os, std::size_t i = 0) const override
    {
      os << indent(i) << "include binary {\n";
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

  class function_expression : public expression
  {
  public:
    inline function_expression (
      const std::string& name,
      const std::vector<std::string>& parameter_list,
      const statement::body& body,
      bool global = false
    ) :
      expression { syntax_type::function_expression },
      m_name { name },
      m_parameter_list { parameter_list },
      m_body { body },
      m_global { global }
    {}

  public:
    inline virtual void dump (std::ostream& os, std::size_t i = 0) const override
    {
      os << indent(i) << (m_global == true ? "global " : "local ") 
         << "function '" << m_name << "' {\n";
      {
        if (m_parameter_list.empty() == false) {
          os << indent(i + 2) << "parameters {\n";
          for (const auto& param : m_parameter_list) {
            os << indent(i + 4) << param << "\n";
          }
          os << indent(i + 2) << "}\n";
        }
        if (m_body.empty() == false) {
          os << indent(i + 2) << "body {\n";
          for (const auto& statement : m_body) {
            statement->dump(os, i + 4);
          }
          os << indent(i + 2) << "}\n";
        }
      }
      os << indent(i) << "}\n";
    }

  public:
    inline const std::string& get_name () const { return m_name; }
    inline const std::vector<std::string>& get_parameter_list () const { return m_parameter_list; }
    inline const statement::body& get_body () const { return m_body; }
    inline bool is_global () const { return m_global; }

  private:
    std::string m_name = "";
    std::vector<std::string> m_parameter_list;
    statement::body m_body;
    bool m_global = false;

  };

  class unary_expression : public expression
  {
  public:
    inline unary_expression (
      const expression::ptr& expr,
      const std::string& oper
    ) :
      expression { syntax_type::unary_expression },
      m_expr { expr },
      m_oper { oper }
    {}

  public:
    inline virtual void dump (std::ostream& os, std::size_t i = 0) const override
    {
      os << indent(i) << "unary expression {\n";
      {
        os << indent(i + 2) << "operator: \"" << m_oper << "\"\n";

        if (m_expr != nullptr) {
          os << indent(i + 2) << "expression:\n";
          m_expr->dump(os, i + 4);
        }
      }
      os << indent(i) << "}\n";
    }

  public:
    inline const expression::ptr& get_expr () const { return m_expr; }
    inline const std::string& get_oper () const { return m_oper; }
  
  private:
    expression::ptr m_expr = nullptr;
    std::string m_oper = "";

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

  class call_expression : public expression
  {
  public:
    inline call_expression (
      const expression::ptr& callee_expr,
      const expression::array& argument_list
    ) :
      expression { syntax_type::call_expression },
      m_callee_expr { callee_expr },
      m_argument_list { argument_list }
    {}

  public:
    inline virtual void dump (std::ostream& os, std::size_t i = 0) const override
    {
      os << indent(i) << "call expression {\n";
      {
        if (m_callee_expr != nullptr) {
          os << indent(i + 2) << "callee\n";
          m_callee_expr->dump(os, i + 4);
        }

        if (m_argument_list.empty() == false) {
          os << indent(i + 2) << "arguments {\n";
          for (const auto& arg : m_argument_list) {
            arg->dump(os, i + 4);
          }
          os << indent(i + 2) << "}\n";
        }
      }
      os << indent(i) << "}\n";
    }

  public:
    inline const expression::ptr& get_callee_expr () const { return m_callee_expr; }
    inline const expression::array& get_argument_list () const { return m_argument_list; }

  private:
    expression::ptr m_callee_expr = nullptr;
    expression::array m_argument_list;

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
      const std::uint64_t integer,
      const double        fractional = 0,
      const std::uint8_t  fraction_bits = 0
    ) :
      expression { syntax_type::numeric_literal },
      m_integer { integer },
      m_fractional { fractional },
      m_fraction_bits { fraction_bits }
    {

    }
  
  public:
    inline virtual void dump (std::ostream& os, std::size_t i = 0) const override
    {
      os << indent(i) << "numeric literal {\n";
      os << indent(i + 2) << "integer: " << m_integer << std::endl;
      os << indent(i + 2) << "fractional: " << m_fractional << std::endl;
      os << indent(i + 2) << "fraction bits: " << (int) m_fraction_bits << std::endl;
      os << indent(i) << "}\n";
    }

  public:
    inline std::uint64_t get_integer () const 
      { return m_integer; }
    inline double get_fractional () const
      { return m_fractional; }
    inline std::uint8_t get_fraction_bits () const
      { return m_fraction_bits; }

  private:
    std::uint64_t m_integer = 0;
    double        m_fractional = 0;
    std::uint8_t  m_fraction_bits = 0;

  };
  
  // class numeric_literal : public expression
  // {
  // public:
  //   inline numeric_literal (
  //     const double value
  //   ) :
  //     expression { syntax_type::numeric_literal },
  //     m_value { value }
  //   {}
  
  // public:
  //   inline virtual void dump (std::ostream& os, std::size_t i = 0) const override
  //   {
  //     os << indent(i) << "numeric literal: " << m_value << "\n";
  //   }
  
  // public:
  //   inline double get_number () const { return m_value; }
  //   inline std::uint64_t get_integer () const { return static_cast<std::uint64_t>(m_value); }
  //   inline std::int64_t get_signed_integer () const { return static_cast<std::int64_t>(m_value); }
  
  // private:
  //   double m_value = 0.0;
      
  // };
  
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
