/** @file smasm/lexer.cpp */

#include <smasm/lexer.hpp>

namespace smasm
{

  bool lexer::lex_file (const fs::path& path)
  {
    fs::path absolute = fs::absolute(path).lexically_normal();

    if (path.extension() != ".asm") {
      std::cerr <<  "[lexer] "
                <<  "File '" << absolute << "' is not an '.asm' file." << std::endl;
      return false;
    }

    if (fs::exists(absolute) == false) {
      std::cerr <<  "[lexer] "
                <<  "File '" << absolute << "' not found." << std::endl;
      return false;
    }

    if (
      std::find(m_paths.begin(), m_paths.end(), absolute) != m_paths.end()
    ) {
    #if defined(SM166_DEBUG)
      std::cout <<  "[lexer] "
                <<  "File '" << absolute << "' was already included." << std::endl;
    #endif
      return true;
    } else {
      m_paths.push_back(absolute);
    }
    
    m_file.open(absolute, std::ios::in);
    if (m_file.is_open() == false) {
      std::cerr <<  "[lexer] "
                <<  "Could not open source file '" << absolute << "' for reading." << std::endl;
      return false;
    }

    m_current_path = absolute;
    m_current_line = 1;
    m_write_ptr = 0;

    if (m_parent_path.empty() == true)
    {
      m_parent_path = m_current_path.parent_path();
    }

    while (true) {
      auto result = collect_token();
      if (result == -1) {
        std::cerr <<  "[lexer] "
                  <<  "  In source file '" << absolute << "':" << m_current_line << "."
                  <<  std::endl;
        m_file.close();
        return false;
      } else if (result == 0) {
        m_file.close();
        return true;
      }
    }

  }

  bool lexer::has_more_tokens () const
  {
    return  m_tokens.empty() == false &&
            m_tokens.at(0).type != token_type::end_of_file;
  }

  token& lexer::token_at (const std::size_t offset)
  {
    if (offset >= m_tokens.size()) {
      std::cerr <<  "[lexer] "
                <<  "Attempt to retrieve token at out-of-range index " << offset << "!"
                <<  std::endl;
      throw std::out_of_range { "Attempt to retrive token at index out of range!" };
    }

    return m_tokens.at(offset);
  }

  const token& lexer::token_at (const std::size_t offset) const
  {
    if (offset >= m_tokens.size()) {
      std::cerr <<  "[lexer] "
                <<  "Attempt to retrieve token at out-of-range index " << offset << "!"
                <<  std::endl;
      throw std::out_of_range { "Attempt to retrive token at index out of range!" };
    }

    return m_tokens.at(offset);
  }

  token lexer::discard_token ()
  {
    token discarded = m_tokens.at(0);
    if (discarded.type != token_type::end_of_file) {
      m_tokens.erase(m_tokens.begin());
    }
    return discarded;
  }

  void lexer::clear_tokens ()
  {
    m_tokens.clear();
  }

  #define smasm_emplace(type, contents) \
    m_tokens.emplace(m_tokens.begin() + m_write_ptr, m_current_path, m_current_line, type, \
      contents); m_write_ptr++;

  int lexer::collect_identifier (std::int32_t& character)
  {
    // Keep a string to hold the contents of the token.
    // Also, keep a lowercase version of the token's contents in this case.
    std::string contents = "", lowercase = "";

    // Collect tokens until a non-alphanumeric, non-underscore character is encountered.
    do {
      contents  += static_cast<char>(character);
      lowercase += static_cast<char>(std::tolower((unsigned char) character));
      character = m_file.get();
    } while (std::isalnum(character) || character == '_');

    // Look up the token's lowercase contents to see if this is a reserved keyword and what kind of
    // keyword that may be. Emplace the appropriate identifier token.
    const auto& kw = keyword::lookup(lowercase);
    switch (kw.type) {
      case keyword_type::language:
      case keyword_type::directive:
      case keyword_type::section:
      case keyword_type::condition:
      case keyword_type::vector:
      case keyword_type::cpu_register:
      case keyword_type::instruction:
        smasm_emplace(token_type::identifier, lowercase);
        break;
      default:
        smasm_emplace(token_type::identifier, contents);
        break;
    }

    // Roll the file stream back one character before returning.
    m_file.unget();
    return 1;

  }

  int lexer::collect_string (std::int32_t& character)
  {

    // Keep a string to hold the token's contents.
    std::string contents = "";

    // Advance the file stream past the opening quotes.
    std::int32_t matching_quotes = character;
    character = m_file.get();

    // Collect characters until the closing matching quote is encountered.
    while (character != matching_quotes) {
      contents  +=  static_cast<char>(character);
      character =   m_file.get();
    }

    // Emplace the string.
    smasm_emplace(token_type::string, contents);
    return 1;

  }

  int lexer::collect_integer (std::int32_t& character)
  {

    // Keep a string to hold the token's contents. Also, keep a boolean flag to indicate whether or
    // not this is an integer or a floating-point number being collected.
    std::string contents = "";
    bool        is_float = false;

    // Collect characters until a non-numeric-character or a second period is encountered.
    do {

      // If a period is encountered, then we know that we are collecting a number token, rather than
      // an integer token.
      //
      // If a second period is encountered while collecting the same token, then we can assume at
      // that point that we have finished collecting this token.
      if (character == '.') {
        if (is_float == true) { break; }
        else { is_float = true; }
      }

      contents  +=  static_cast<char>(character);
      character =   m_file.get();

    } while (std::isdigit(character) || character == '.');

    // Emplace an integer or a floating-point numeric token.
    smasm_emplace(is_float == true ? token_type::number : token_type::integer, contents);

    m_file.unget();
    return 1;

  }

  int lexer::collect_hexadecimal (std::int32_t& character)
  {
    std::string contents = "";
    character = m_file.get();

    while (std::isxdigit(character)) {
      contents  += static_cast<char>(character);
      character  = m_file.get();
    }

    m_file.unget();
    if (contents == "") { smasm_emplace(token_type::dollar, "$"); }
    else { smasm_emplace(token_type::hexadecimal, contents); }
    
    return 1;
  }

  int lexer::collect_binary (std::int32_t& character)
  {
    std::string contents = "";
    character = m_file.get();

    while (character == '0' || character == '1') {
      contents  += static_cast<char>(character);
      character  = m_file.get();
    }

    m_file.unget();
    if (contents == "") { smasm_emplace(token_type::percent, "%"); }
    else { smasm_emplace(token_type::binary, contents); }
    
    return 1;
  }

  int lexer::collect_octal (std::int32_t& character)
  {

    std::string contents = "";
    character = m_file.get();

    while (character >= '0' && character <= '7') {
      contents  += static_cast<char>(character);
      character  = m_file.get();
    }

    m_file.unget();
    if (contents == "") { 
      std::int32_t next_character = m_file.get();
      if (next_character == '&') {
        smasm_emplace(token_type::double_ampersand, "&&");
      } else {
        m_file.unget();
        smasm_emplace(token_type::ampersand, "&"); 
      }
    }
    else { smasm_emplace(token_type::octal, contents); }
    
    return 1;

  }

  int lexer::collect_symbol (std::int32_t& character)
  {
    // Keep track of the type of the symbol token being collected.
    token_type type = token_type::unknown;

    // Based on the current character, determine which symbol token is being collected.
    switch (character)
    {
      case '`':  type =    token_type::backtick; break;
      case '?':  type =    token_type::question; break;
      case '!': {
        std::int32_t next_character = m_file.get();
        if (next_character == '=') {
          smasm_emplace(token_type::not_equals, "!=");
          return 1;
        } else {
          m_file.unget();
          smasm_emplace(token_type::exclaim, "!");
          return 1;
        }
      } break;
      case '.':  type =    token_type::period; break;
      case ',':  type =    token_type::comma; break;
      case ':':  type =    token_type::colon; break;
      case '@':  type =    token_type::at; break;
      case '#':  type =    token_type::pound; break;
      case '^':  type =    token_type::carat; break;
      case '|': {
        std::int32_t next_character = m_file.get();
        if (next_character == '|') {
          smasm_emplace(token_type::double_pipe, "||");
          return 1;
        } else {
          m_file.unget();
          smasm_emplace(token_type::pipe, "|"); 
          return 1;
        }
      } break;
      case '*':  type =    token_type::asterisk; break;
      case '+':  type =    token_type::plus; break;
      case '-':  type =    token_type::minus; break;
      case '=': {
        std::int32_t next_character = m_file.get();
        if (next_character == '=') {
          smasm_emplace(token_type::double_equals, "==");
          return 1;
        } else {
          m_file.unget();
          smasm_emplace(token_type::equals, "=");
          return 1;
        }
      } break;
      case '/':  type =    token_type::slash; break;
      case '\\': type =    token_type::backslash; break;
      case '(':  type =    token_type::open_paren; break;
      case ')':  type =    token_type::close_paren; break;
      case '[':  type =    token_type::open_bracket; break;
      case ']':  type =    token_type::close_bracket; break;
      case '{':  type =    token_type::open_brace; break;
      case '}':  type =    token_type::close_brace; break;
      case '<': {
        std::int32_t next_character = m_file.get();
        if (next_character == '=') {
          smasm_emplace(token_type::less_equals, "<=");
          return 1;
        } else if (next_character == '<') {
          smasm_emplace(token_type::left_shift, "<<");
          return 1;
        } else {
          m_file.unget();
          smasm_emplace(token_type::open_arrow, "<");
          return 1;
        }
      } break;
      case '>': {
        std::int32_t next_character = m_file.get();
        if (next_character == '=') {
          smasm_emplace(token_type::greater_equals, ">=");
          return 1;
        } else if (next_character == '>') {
          smasm_emplace(token_type::right_shift, ">>");
          return 1;
        } else {
          m_file.unget();
          smasm_emplace(token_type::close_arrow, ">");
          return 1;
        }
      } break;
      default:
        std::cerr << "[lexer] "
                  << "Unexpected character '" << static_cast<char>(character) << "'."
                  << std::endl;
        return -1;
    }

    // Emplace the deduced token.
    smasm_emplace(type, std::string { (char) character });
    return 1;
  }

  int lexer::collect_token ()
  {

    // Retrieve the next character in the file stream. It needs to be an `std::int32_t` because the
    // end-of-file marker (`EOF`) needs to fit in here, too.
    std::int32_t character = m_file.get();

    // If a whitespace character (like a space ' ' or a newline '\n') is encountered, then skip over
    // it and get the next character. Repeat this until a non-whitespace character is encountered.
    while (std::isspace(character)) 
    {
      // If that whitespace character is, in fact, the newline character, then also increment the
      // line counter.
      if (character == '\n') { 
        m_current_line++;
        return 1;
      }

      character = m_file.get();
    }

    // If the character encountered is a semicolon ';', then this is the start of a comment.
    // Ignore all characters from that point until the end o fthe current line.
    if (character == ';') {
      while (character != '\n' && character != std::char_traits<char>::eof()) {
        character = m_file.get();
      }

      if (character == '\n') {
        m_current_line++;
        return 1;
      } else if (character == std::char_traits<char>::eof()) {
        smasm_emplace(token_type::end_of_file, "");
        return 0;
      }
    }

    // Next, check for the end-of-file marker (`EOF`). In C++, there is a character traits
    // specialization for this marker.
    if (character == std::char_traits<char>::eof()) {
      smasm_emplace(token_type::end_of_file, "");
      return 0;
    }

    if (std::isalpha(character) || character == '_') {
      return collect_identifier(character);
    } else if (character == '"') {
      return collect_string(character);
    } else if (std::isdigit(character)) {
      return collect_integer(character);
    } else if (character == '$') {
      return collect_hexadecimal(character);
    } else if (character == '%') {
      return collect_binary(character);
    } else if (character == '&') {
      return collect_octal(character);
    } else  {
      return collect_symbol(character);
    }

  }
  
}
