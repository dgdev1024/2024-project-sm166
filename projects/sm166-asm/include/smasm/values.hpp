/** @file smasm/values.hpp */

#pragma once

#include <smasm/syntax.hpp>

namespace smasm
{

  enum class value_type
  {
    none,
    cpu_register,
    cpu_condition,
    address,
    number,
    string,
    function
  };

  /** Runtime Value Base Class ********************************************************************/

  class value
  {
  public:
    using ptr = std::shared_ptr<value>;

  protected:
    inline value (
      const value_type type
    ) :
      m_value_type { type }
    {}

  public:
    template <typename T, typename... As>
    inline static std::shared_ptr<T> make (As&&... args)
    {
      static_assert(std::is_base_of_v<value, T>, "'T' must derive from 'smasm::value'.");
      return std::make_shared<T>(std::forward<As>(args)...);
    }

  public:
    inline value_type get_value_type () const { return m_value_type; }

  protected:
    value_type m_value_type;

  };

  /** Value Casting Function **********************************************************************/

  template <typename T>
  inline std::shared_ptr<T> value_cast (const value::ptr& ptr)
  {
    static_assert(std::is_base_of_v<value, T>, "'T' must derive from 'smasm::value'.");
    return std::static_pointer_cast<T>(ptr);
  }

  /** Basic Value Types ***************************************************************************/

  class void_value : public value
  {
  public:
    inline void_value () :
      value { value_type::none }
    {}

  };

  class cpu_register_value : public value
  {
  public:
    inline cpu_register_value (
      const register_type type,
      bool is_address_ptr = false
    ) :
      value             { value_type::cpu_register },
      m_type            { type },
      m_is_address_ptr  { is_address_ptr }
    {}

  public:
    inline register_type get_type () const { return m_type; }
    inline bool is_byte_register () const 
      { return m_type < register_type::rt_w0; }
    inline bool is_word_register () const
      { return m_type >= register_type::rt_w0 && m_type < register_type::rt_l0; }
    inline bool is_long_register () const
      { return m_type >= register_type::rt_l0; }
    inline bool is_address_pointer () const
      { return m_is_address_ptr; }

  private:
    register_type m_type;
    bool m_is_address_ptr = false;

  };

  class cpu_condition_value : public value
  {
  public:
    inline cpu_condition_value (
      const condition_type type
    ) :
      value   { value_type::cpu_condition },
      m_type  { type }
    {}

  public:
    inline condition_type get_type () const { return m_type; }

  private:
    condition_type m_type;

  };

  class address_value : public value
  {
  public:
    inline address_value (
      const std::uint64_t address
    ) :
      value { value_type::address },
      m_address { static_cast<std::uint32_t>(address & 0xFFFFFFFF) }
    {}

  public:
    inline std::uint32_t get_address () const { return m_address; }

  private:
    std::uint32_t m_address = 0;

  };
  
  class number_value : public value
  {
  public:
    inline number_value (
      const double val
    ) :
      value { value_type::number },
      m_value { val }
    {}

  public:
    inline double get_number () const { return m_value; }
    inline std::uint64_t get_integer () const { return static_cast<std::uint64_t>(m_value); }

  private:
    double m_value = 0.0;

  };
  
  class string_value : public value
  {
  public:
    inline string_value (
      const std::string& val
    ) :
      value { value_type::string },
      m_value { val }
    {}

  public:
    inline const std::string& get_string () const { return m_value; }

  private:
    std::string m_value = "";

  };

  /* Function Values ******************************************************************************/

  class function_value : public value
  {
  public:
    inline function_value (
      const std::string& name,
      const std::vector<std::string>& parameter_list,
      const statement::body& body
    ) :
      value { value_type::function },
      m_name { name },
      m_parameter_list { parameter_list },
      m_body { body }
    {}

  public:
    inline const std::string& get_name () const { return m_name; }
    inline const std::vector<std::string>& get_parameter_list () const { return m_parameter_list; }
    inline const statement::body& get_body () const { return m_body; }

  private:
    std::string m_name = "";
    std::vector<std::string> m_parameter_list;
    statement::body m_body;

  };

}
