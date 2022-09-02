#include "lexer.h"

#include <algorithm>
#include <charconv>
#include <unordered_map>

using namespace std;

namespace parse {

	bool operator==(const Token& lhs, const Token& rhs) {
		using namespace token_type;

		if (lhs.index() != rhs.index()) {
			return false;
		}
		if (lhs.Is<Char>()) {
			return lhs.As<Char>().value == rhs.As<Char>().value;
		}
		if (lhs.Is<Number>()) {
			return lhs.As<Number>().value == rhs.As<Number>().value;
		}
		if (lhs.Is<String>()) {
			return lhs.As<String>().value == rhs.As<String>().value;
		}
		if (lhs.Is<Id>()) {
			return lhs.As<Id>().value == rhs.As<Id>().value;
		}
		return true;
	}

	bool operator!=(const Token& lhs, const Token& rhs) {
		return !(lhs == rhs);
	}

	std::ostream& operator<<(std::ostream& os, const Token& rhs) {
		using namespace token_type;

#define VALUED_OUTPUT(type) \
    if (auto p = rhs.TryAs<type>()) return os << #type << '{' << p->value << '}';

		VALUED_OUTPUT(Number);
		VALUED_OUTPUT(Id);
		VALUED_OUTPUT(String);
		VALUED_OUTPUT(Char);

#undef VALUED_OUTPUT

#define UNVALUED_OUTPUT(type) \
    if (rhs.Is<type>()) return os << #type;

		UNVALUED_OUTPUT(Class);
		UNVALUED_OUTPUT(Return);
		UNVALUED_OUTPUT(If);
		UNVALUED_OUTPUT(Else);
		UNVALUED_OUTPUT(Def);
		UNVALUED_OUTPUT(Newline);
		UNVALUED_OUTPUT(Print);
		UNVALUED_OUTPUT(Indent);
		UNVALUED_OUTPUT(Dedent);
		UNVALUED_OUTPUT(And);
		UNVALUED_OUTPUT(Or);
		UNVALUED_OUTPUT(Not);
		UNVALUED_OUTPUT(Eq);
		UNVALUED_OUTPUT(NotEq);
		UNVALUED_OUTPUT(LessOrEq);
		UNVALUED_OUTPUT(GreaterOrEq);
		UNVALUED_OUTPUT(None);
		UNVALUED_OUTPUT(True);
		UNVALUED_OUTPUT(False);
		UNVALUED_OUTPUT(Eof);

#undef UNVALUED_OUTPUT

		return os << "Unknown token :("sv;
	}

	Lexer::Lexer(std::istream& input) :istrm_(input), current_token_(parse::token_type::None{}) {
		current_token_ = NextToken();
	}

	const Token& Lexer::CurrentToken() const {
		// Заглушка. Реализуйте метод самостоятельно
		return current_token_;

		//throw std::logic_error("Not implemented"s);
	}

	void IgnoreSpaces(std::istream& input) {
		while (input.peek() == ' ') {
			input.get();
		}
	}

	Token& Lexer::ParseDigit() {
		parse::token_type::Number number_type;
		istrm_ >> number_type.value;
		current_token_ = number_type;
		IgnoreSpaces(istrm_);
		return current_token_;
	}

	Token& Lexer::ParseString(char quote_type) {
		string str_line;
		char ch = istrm_.get();
		while (ch != quote_type) {
			if (ch == '\\') {
				char ch_next = istrm_.peek();

				switch (ch_next) {
				case '\'':str_line.push_back(istrm_.get()); break;
				case '\"':str_line.push_back(istrm_.get()); break;
				case '\\':str_line.push_back(istrm_.get()); break;
				case 'n':str_line.push_back('\n'); istrm_.get(); break;
				case 't':str_line.push_back('\t'); istrm_.get(); break;
				case 'r':str_line.push_back('\r'); istrm_.get(); break;
				default:
					break;
				}
			}
			else {
				str_line.push_back(ch);
			}
			ch = istrm_.get();
		}
		parse::token_type::String string_type;
		string_type.value = str_line;
		current_token_ = string_type;
		IgnoreSpaces(istrm_);
		return current_token_;
	}

	Token& Lexer::ParseId(char ch) {
		string id_str;
		id_str.push_back(ch);
		while (istrm_.peek() != ' '
			&& !(ispunct(istrm_.peek()) && istrm_.peek() != '_')
			&& istrm_.peek() != '\n'
			&& istrm_.peek() != EOF) {
			id_str.push_back(istrm_.get());
		}
		if (id_str == "class") { current_token_ = parse::token_type::Class{}; }
		else if (id_str == "return") { current_token_ = parse::token_type::Return{}; }
		else if (id_str == "if") { current_token_ = parse::token_type::If{}; }
		else if (id_str == "else") { current_token_ = parse::token_type::Else{}; }
		else if (id_str == "def") { current_token_ = parse::token_type::Def{}; }
		else if (id_str == "print") { current_token_ = parse::token_type::Print{}; }
		else if (id_str == "and") { current_token_ = parse::token_type::And{}; }
		else if (id_str == "or") { current_token_ = parse::token_type::Or{}; }
		else if (id_str == "not") { current_token_ = parse::token_type::Not{}; }
		else if (id_str == "None") { current_token_ = parse::token_type::None{}; }
		else if (id_str == "True") { current_token_ = parse::token_type::True{}; }
		else if (id_str == "False") { current_token_ = parse::token_type::False{}; }
		else {
			parse::token_type::Id id_type;
			id_type.value = id_str;
			current_token_ = id_type;
		}
		IgnoreSpaces(istrm_);
		return current_token_;
	}

	Token& Lexer::ParsePunct(char ch) {
		if (ch == '=' && istrm_.peek() == '=') {
			istrm_.get();
			current_token_ = parse::token_type::Eq{};
		}
		else if (ch == '!' && istrm_.peek() == '=') {
			istrm_.get();
			current_token_ = parse::token_type::NotEq{};
		}
		else if (ch == '<' && istrm_.peek() == '=') {
			istrm_.get();
			current_token_ = parse::token_type::LessOrEq{};
		}
		else if (ch == '>' && istrm_.peek() == '=') {
			istrm_.get();
			current_token_ = parse::token_type::GreaterOrEq{};
		}
		else if (ch == '#') {
			while (istrm_.peek() != '\n' && !istrm_.eof()) {
				istrm_.get();
			}
			current_token_ = NextToken();
		}
		else {
			parse::token_type::Char char_type;
			char_type.value = ch;
			current_token_ = char_type;
		}
		IgnoreSpaces(istrm_);
		return current_token_;
	}

	Token Lexer::NextToken() {
		using namespace parse::token_type;
		char ch = istrm_.get();
		while (ch != EOF) {
			if (ch == '\n') {
				if (current_token_.Is<None>() || current_token_.Is<Newline>()) {
					return NextToken();
				}
				parse::token_type::Newline newline_type;
				current_token_ = newline_type;
				return current_token_;
			}

			int indent_pos_this = 0;
			if (current_token_.Is<Newline>()) {
				while (ch == ' ' && istrm_.peek() == ' ') {
					istrm_.get();
					indent_pos_this++;
					ch = istrm_.get();
				}
				indent_offset_ = indent_pos_this - indent_pos_;
			}

			if (indent_offset_ > 0) {
				istrm_.putback(ch);
				indent_pos_++;
				indent_offset_--;
				current_token_ = Indent{};
				return current_token_;
			}
			else if (indent_offset_ < 0) {
				istrm_.putback(ch);
				indent_pos_--;
				indent_offset_++;
				current_token_ = Dedent{};
				return current_token_;
			}
			else {
				if (isdigit(ch)) {
					istrm_.putback(ch);
					return ParseDigit();
				}
				else if (ch == '\'' || ch == '\"') {
					return ParseString(ch);
				}
				else if (isalpha(ch) || ch == '_') {
					return ParseId(ch);
				}
				else if (ispunct(ch) && ch != '_') {
					return ParsePunct(ch);
				}
				/*else if (ch == '\n') {
					if (current_token_.Is<None>() || current_token_.Is<Newline>()) {
						return NextToken();
					}
					parse::token_type::Newline newline_type;
					current_token_ = newline_type;
					return current_token_;
				}*/
				else if (ch == EOF) {
					parse::token_type::Eof eof_type;
					current_token_ = eof_type;
					return current_token_;
				}
			}
		}
		if (indent_pos_ > 0) {
			indent_pos_--;
			current_token_ = Dedent{};
			return current_token_;
		}
		if (current_token_.Is<None>() || current_token_.Is<Eof>() || current_token_.Is<Dedent>()) {
			current_token_ = Eof{};
			return current_token_;
		}
		else if (!current_token_.Is<None>() && !current_token_.Is<Newline>()) {
			current_token_ = Newline{};
			return current_token_;
		}
		current_token_ = Eof{};
		return current_token_;

		throw std::logic_error("Not implemented"s);
	}

}  // namespace parse

// 'word' "two words" 'long string with a double quote " inside' "another long string with single quote ' inside" "\"\'\t\n"
// "\"\'\t\n"