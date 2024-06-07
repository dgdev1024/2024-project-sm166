/** @file smasm/main.cpp */

#include <smasm/arguments.hpp>
#include <smasm/lexer.hpp>
#include <smasm/parser.hpp>
#include <smasm/assembly.hpp>
#include <smasm/interpreter.hpp>

int main (int argc, char** argv)
{
  if (smasm::arguments::parse(argc, argv) == false) {
    return 1;
  }

  auto input_file = smasm::arguments::get("input-filename", 'i');
  if (input_file.empty()) {
    std::cerr << "Missing input filename argument (--input-filename, -i)." << std::endl;
    return 1;
  }

  auto output_file = smasm::arguments::get("output-file", 'o');
  if (
    smasm::arguments::has("output-file", 'o') == false &&
    smasm::arguments::has("lex-only", 'l') == false &&
    smasm::arguments::has("ast-only", 's') == false &&
    smasm::arguments::has("no-output", 'n') == false
  ) {
    std::cerr << "Missing output filename argument (--output-filename, -o)." << std::endl;
    return 1;
  }

  smasm::lexer lexer;
  if (lexer.lex_file(input_file) == false) {
    return 1;
  }

  if (smasm::arguments::has("lex-only", 'l')) {
    std::size_t index = 0;

    while (lexer.has_more_tokens() == true) {
      auto token = lexer.discard_token();
      std::cout << (++index) << ". '" << token.get_string_type() << "' = '" << token.contents
                << "'" << std::endl;
    }

    return 0;
  }

  smasm::parser parser;
  smasm::program::ptr program = parser.parse_program(lexer);
  if (program == nullptr) {
    return 1;
  }

  if (smasm::arguments::has("ast-only", 's')) {
    program->dump(std::cout);
    return 0;
  }

  smasm::assembly assembly;
  smasm::environment environment;
  smasm::interpreter interpreter { lexer, assembly, environment };
  if (interpreter.evaluate(program) == nullptr) {
    return 1;
  }

  if (smasm::arguments::has("no-output", 'n') == false && assembly.save_rom(output_file) == false) {
    return 1;
  }

  return 0;
}
