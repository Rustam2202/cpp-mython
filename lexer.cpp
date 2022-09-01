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
		// Реализуйте конструктор самостоятельно

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

	bool Lexer::CheckDedent() {
		if (spaces_count_ > 0) {
			spaces_count_ -= 2;
			current_token_ = parse::token_type::Dedent{};
			return true;
		}
		else {
			return false;
		}
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

			uint32_t spaces_count_new = 0;
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
				if (ch == '\'') {
					//istrm_.putback(ch);
					string str_line;
					getline(istrm_, str_line, '\'');
					parse::token_type::String string_type;
					string_type.value = str_line;
					current_token_ = string_type;
					IgnoreSpaces(istrm_);
					return current_token_;
				}
				else if (ch == '\"') {
					//istrm_.putback(ch);
					string str_line;
					getline(istrm_, str_line, '\"');
					parse::token_type::String string_type;
					string_type.value = str_line;
					current_token_ = string_type;
					IgnoreSpaces(istrm_);
					return current_token_;
				}
				else if (isdigit(ch)) {
					istrm_.putback(ch);
					parse::token_type::Number number_type;
					istrm_ >> number_type.value;
					current_token_ = number_type;
					IgnoreSpaces(istrm_);
					return current_token_;
				}
				else if (ispunct(ch)) {
					if (ch == '_' && isalnum(istrm_.peek())) {
						istrm_.putback(ch);
						parse::token_type::Id id_type;
						istrm_ >> id_type.value;
						current_token_ = id_type;
						IgnoreSpaces(istrm_);
						return current_token_;
					}
					else if (ispunct(istrm_.peek())) {
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
						else {
							parse::token_type::Char char_type;
							char_type.value = ch;
							current_token_ = char_type;
						}
					}
					else {
						parse::token_type::Char char_type;
						char_type.value = ch;
						current_token_ = char_type;
					}

					IgnoreSpaces(istrm_);
					return current_token_;
				}
				else if (ch == '\n') {
					if (current_token_.Is<None>() || current_token_.Is<Newline>()) {
						return NextToken();
					}
					parse::token_type::Newline newline_type;
					current_token_ = newline_type;
					return current_token_;
				}
				else if (isalpha(ch) && !isdigit(ch)) {
					//istrm_.putback(ch);
					string type;
					//istrm_ >> type;
					type.push_back(ch);
					while (true) {
						if ((ispunct(istrm_.peek()) && istrm_.peek() != '_') || isspace(istrm_.peek()) || istrm_.peek() == '\n' || istrm_.peek() == EOF) {
							break;
						}
						type.push_back(istrm_.get());
					}

					if (type == "class") {
						current_token_ = parse::token_type::Class{};
					}
					else if (type == "return") {
						current_token_ = parse::token_type::Return{};
					}
					else if (type == "if") {
						current_token_ = parse::token_type::If{};
					}
					else if (type == "else") {
						current_token_ = parse::token_type::Else{};
					}
					else if (type == "def") {
						current_token_ = parse::token_type::Def{};
					}
					else if (type == "print") {
						current_token_ = parse::token_type::Print{};
					}
					else if (type == "and") {
						current_token_ = parse::token_type::And{};
					}
					else if (type == "or") {
						current_token_ = parse::token_type::Or{};
					}
					else if (type == "not") {
						current_token_ = parse::token_type::Not{};
					}
					else if (type == "None") {
						current_token_ = parse::token_type::None{};
					}
					else if (type == "True") {
						current_token_ = parse::token_type::True{};
					}
					else if (type == "False") {
						current_token_ = parse::token_type::False{};
					}
					else {
						parse::token_type::Id id_type;
						id_type.value = type;
						current_token_ = id_type;
					}
					IgnoreSpaces(istrm_);
					return current_token_;

				}
				else if (!isalpha(ch) && ch != '\n') {
					//istrm_.putback(ch);
					parse::token_type::Char char_type;
					char_type.value = ch;
					current_token_ = char_type;
					IgnoreSpaces(istrm_);
					return current_token_;
				}
				else if (ch == EOF) {
					parse::token_type::Eof eof_type;
					current_token_ = eof_type;
					return current_token_;
				}

			}
			//		throw std::logic_error("Not implemented"s);
		}
		if (indent_pos_ > 0) {
			indent_pos_--;
			current_token_ = Dedent{};
			return current_token_;
		}
		parse::token_type::Eof eof_type;
		current_token_ = eof_type;
		return current_token_;
	}

}  // namespace parse