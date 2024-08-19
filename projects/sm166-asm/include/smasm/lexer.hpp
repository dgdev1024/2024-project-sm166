/** @file smasm/lexer.hpp */

#pragma once

#include <smasm/token.hpp>

namespace smasm
{

  class lexer
  {
  public:
    using ptr = std::shared_ptr<lexer>;

  public:
    bool lex_file (const fs::path& path);
    bool has_more_tokens () const;
    token& token_at (const std::size_t offset = 0);
    const token& token_at (const std::size_t offset = 0) const;
    token discard_token ();
    void clear_tokens ();
    void clear_includes ();

  public:
    
    inline const std::vector<token>& get_tokens () const
    {
      return m_tokens;
    }

    inline const fs::path& get_path () const
    {
      return m_current_path;
    }

    inline const fs::path& get_parent_path () const
    {
      return m_parent_path;
    }

  private:
    int collect_identifier (std::int32_t& character);
    int collect_string (std::int32_t& character);
    int collect_integer (std::int32_t& character);
    int collect_hexadecimal (std::int32_t& character);
    int collect_binary (std::int32_t& character);
    int collect_octal (std::int32_t& character);
    int collect_symbol (std::int32_t& character);
    int collect_token ();

  private:
    std::fstream m_file;
    std::vector<token> m_tokens;
    std::vector<fs::path> m_paths;
    fs::path m_current_path = "";
    fs::path m_parent_path = "";
    std::size_t m_current_line = 0;
    std::size_t m_write_ptr = 0;

  };

}
