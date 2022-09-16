#include "statement.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace ast {

	using runtime::Closure;
	using runtime::Context;
	using runtime::ObjectHolder;

	namespace {
		const string ADD_METHOD = "__add__"s;
		const string INIT_METHOD = "__init__"s;
		const string STR_METHOD = "__str__"s;
	}  // namespace

	ObjectHolder Assignment::Execute(Closure& closure, Context& context) {
		// Заглушка. Реализуйте метод самостоятельно
		closure[name_] = rv_.get()->Execute(closure, context);
		//closure[name_] = std::move(rv_.get()->Execute(closure, context));
		return closure.at(name_);
	}

	Assignment::Assignment(std::string var, std::unique_ptr<Statement> rv) {
		name_ = var;
		rv_ = std::move(rv);
	}

	VariableValue::VariableValue(const std::string& var_name) {
		name_ = var_name;
	}

	VariableValue::VariableValue(std::vector<std::string> dotted_ids) :dotted_ids_(dotted_ids) {
		name_ = dotted_ids.back();
	}

	FieldAssignment::FieldAssignment(VariableValue object, std::string field_name, std::unique_ptr<Statement> rv)
		:object_(object), field_name_(field_name), rv_(std::move(rv))
	{
		//object_ = std::move(object);
		//field_name_ = field_name;
		//rv_ = std::move(rv);
	}

	ObjectHolder FieldAssignment::Execute(Closure& closure, Context& context) {
		// Заглушка. Реализуйте метод самостоятельно

		runtime::ClassInstance* cls = object_.Execute(closure, context).TryAs<runtime::ClassInstance>();
		if (rv_) {
			cls->Fields()[field_name_] = rv_.get()->Execute(closure, context);
		}
		return cls->Fields().at(field_name_);
	}

	ObjectHolder VariableValue::Execute(Closure& closure, Context& context) {
		// Заглушка. Реализуйте метод самостоятельно
		using namespace runtime;

		if (dotted_ids_.size() == 1) {
			return closure[dotted_ids_[0]];
			//return closure.at(name_);
		}
		if (!dotted_ids_.empty()) {
			return closure.at(dotted_ids_[0]).TryAs<ClassInstance>()->Fields().at(dotted_ids_[1]);
		}
		else
			if (closure.count(name_)) {
				return closure.at(name_);
			}
			else {
				throw std::runtime_error("Not implemented"s);
			}
		return {};
	}

	unique_ptr<Print> Print::Variable(const std::string& name) {
		// Заглушка, реализуйте метод самостоятельно

		return make_unique<Print>(Print{ make_unique<VariableValue>(name) });

		//	throw std::logic_error("Not implemented"s);
	}

	Print::Print(unique_ptr<Statement> argument) {
		// Заглушка, реализуйте метод самостоятельно
		args_.push_back(std::move(argument));
	}

	Print::Print(vector<unique_ptr<Statement>> args) :args_(std::move(args)) {
		// Заглушка, реализуйте метод самостоятельно
	}


	ObjectHolder Print::Execute(Closure& closure, Context& context) {
		// Заглушка. Реализуйте метод самостоятельно
		auto it = args_.begin();
		while (true) {
			auto a = (*it).get()->Execute(closure, context);
			if (a) {
				a.Get()->Print(context.GetOutputStream(), context);
			}
			else {
				context.GetOutputStream() << "None";
			}

			if (it == args_.end() - 1) {
				context.GetOutputStream() << '\n';
				break;
			}
			else {
				context.GetOutputStream() << ' ';
			}
			it++;
		}

		return {};
	}

	MethodCall::MethodCall(std::unique_ptr<Statement> /*object*/, std::string /*method*/, std::vector<std::unique_ptr<Statement>> /*args*/) {
		// Заглушка. Реализуйте метод самостоятельно
	}

	ObjectHolder MethodCall::Execute(Closure& /*closure*/, Context& /*context*/) {
		// Заглушка. Реализуйте метод самостоятельно
		return {};
	}

	ObjectHolder Stringify::Execute(Closure& closure, Context& context) {
		// Заглушка. Реализуйте метод самостоятельно

		std::string str;

		auto value = GetArg().get()->Execute(closure, context);
		if (value.TryAs<runtime::Number>()) {
			str = std::to_string(value.TryAs<runtime::Number>()->GetValue());
		}
		else if (value.TryAs<runtime::Bool>()) {
			bool v = value.TryAs<runtime::Bool>()->GetValue();
			v ? str = "True"s : str = "False"s;
		}
		else if (value.TryAs<runtime::ClassInstance>()) {
			auto cls = value.TryAs<runtime::ClassInstance>();
			if (cls->HasMethod(STR_METHOD, 0)) {
				ObjectHolder c = cls->Call(STR_METHOD, {}, context);
				str = std::to_string(c.TryAs<runtime::Number>()->GetValue());
			}
			else {
				return this->GetArg().get()->Execute(closure, context);
			}
		}
		else if (!value) {
			str = "None"s;
		}
		else {
			str = value.TryAs<runtime::String>()->GetValue();
		}
		return ObjectHolder::Own(runtime::String(str));
	}

	ObjectHolder Add::Execute(Closure& closure, Context& context) {
		// Заглушка. Реализуйте метод самостоятельно

		ObjectHolder left = GetLhs().get()->Execute(closure, context);
		ObjectHolder right = GetRhs().get()->Execute(closure, context);

		if (left.TryAs<runtime::Number>() && right.TryAs<runtime::Number>()) {
			int numb = left.TryAs<runtime::Number>()->GetValue() + right.TryAs<runtime::Number>()->GetValue();
			return ObjectHolder::Own(runtime::Number(numb));
		}
		else if (left.TryAs<runtime::String>() && right.TryAs<runtime::String>()) {
			std::string str = left.TryAs<runtime::String>()->GetValue() + right.TryAs<runtime::String>()->GetValue();
			return ObjectHolder::Own(runtime::String(str));
		}
		else if (left.TryAs<runtime::ClassInstance>()) {
			if (left.TryAs<runtime::ClassInstance>()->HasMethod(ADD_METHOD, 1)) {
				return left.TryAs<runtime::ClassInstance>()->Call(ADD_METHOD, { right }, context);
			}
			else {
				throw std::runtime_error("Not implemented"s);
			}
		}
		else {
			throw std::runtime_error("Not implemented"s);
		}
	}

	ObjectHolder Sub::Execute(Closure& closure, Context& context) {
		// Заглушка. Реализуйте метод самостоятельно

		ObjectHolder left = GetLhs().get()->Execute(closure, context);
		ObjectHolder right = GetRhs().get()->Execute(closure, context);

		if (left.TryAs<runtime::Number>() && right.TryAs<runtime::Number>()) {
			int numb = left.TryAs<runtime::Number>()->GetValue() - right.TryAs<runtime::Number>()->GetValue();
			return ObjectHolder::Own(runtime::Number(numb));
		}
		else {
			throw std::runtime_error("Not implemented"s);
		}
	}

	ObjectHolder Mult::Execute(Closure& closure, Context& context) {
		// Заглушка. Реализуйте метод самостоятельно

		ObjectHolder left = GetLhs().get()->Execute(closure, context);
		ObjectHolder right = GetRhs().get()->Execute(closure, context);

		if (left.TryAs<runtime::Number>() && right.TryAs<runtime::Number>()) {
			int numb = left.TryAs<runtime::Number>()->GetValue() * right.TryAs<runtime::Number>()->GetValue();
			return ObjectHolder::Own(runtime::Number(numb));
		}
		else {
			throw std::runtime_error("Not implemented"s);
		}
	}

	ObjectHolder Div::Execute(Closure& closure, Context& context) {
		// Заглушка. Реализуйте метод самостоятельно

		ObjectHolder left = GetLhs().get()->Execute(closure, context);
		ObjectHolder right = GetRhs().get()->Execute(closure, context);

		if (left.TryAs<runtime::Number>() && right.TryAs<runtime::Number>()) {
			if (right.TryAs<runtime::Number>()->GetValue() == 0) {
				throw std::runtime_error("Not implemented"s);
			}
			int numb = left.TryAs<runtime::Number>()->GetValue() / right.TryAs<runtime::Number>()->GetValue();
			return ObjectHolder::Own(runtime::Number(numb));
		}
		else {
			throw std::runtime_error("Not implemented"s);
		}
	}

	ObjectHolder Compound::Execute(Closure& closure, Context& context) {
		// Заглушка. Реализуйте метод самостоятельно

		for (auto& arg : args_) {
			arg.Execute(closure, context);
		}
		//closure = closure_;
		return ObjectHolder::None();
	}

	ObjectHolder Return::Execute(Closure& /*closure*/, Context& /*context*/) {
		// Заглушка. Реализуйте метод самостоятельно
		return {};
	}

	ClassDefinition::ClassDefinition(ObjectHolder /*cls*/) {
		// Заглушка. Реализуйте метод самостоятельно
	}

	ObjectHolder ClassDefinition::Execute(Closure& /*closure*/, Context& /*context*/) {
		// Заглушка. Реализуйте метод самостоятельно
		return {};
	}


	IfElse::IfElse(std::unique_ptr<Statement> /*condition*/, std::unique_ptr<Statement> /*if_body*/, std::unique_ptr<Statement> /*else_body*/) {
		// Реализуйте метод самостоятельно
	}

	ObjectHolder IfElse::Execute(Closure& /*closure*/, Context& /*context*/) {
		// Заглушка. Реализуйте метод самостоятельно
		return {};
	}

	ObjectHolder Or::Execute(Closure& closure, Context& context) {
		// Заглушка. Реализуйте метод самостоятельно

		ObjectHolder left = GetLhs().get()->Execute(closure, context);
		ObjectHolder right = GetRhs().get()->Execute(closure, context);
		bool left_bool = false;
		bool right_bool = false;

		if (left.TryAs<runtime::Bool>() && right.TryAs<runtime::Bool>()) {
			if (left_bool = left.TryAs<runtime::Bool>()->GetValue() == false) {
				right_bool = right.TryAs<runtime::Bool>()->GetValue();
				return right;
			}
			return left;
		}
		/*else if (left.TryAs<runtime::Number>() && right.TryAs<runtime::Number>()) {
			if (left_bool = left.TryAs<runtime::Number>()->GetValue() == false) {
				right_bool = right.TryAs<runtime::Number>()->GetValue();
				return right;
			}
			return left;
		}
		else if (left.TryAs<runtime::String>() && right.TryAs<runtime::String>()) {
			if (left_bool = left.TryAs<runtime::String>()->GetValue().empty()) {
				right_bool = right.TryAs<runtime::String>()->GetValue().empty();
				return right;
			}
			return left;
		}*/
		else {
			throw std::runtime_error("Not implemented"s);
		}
	}

	ObjectHolder And::Execute(Closure& closure, Context& context) {
		// Заглушка. Реализуйте метод самостоятельно

		ObjectHolder left = GetLhs().get()->Execute(closure, context);
		ObjectHolder right = GetRhs().get()->Execute(closure, context);
		bool left_bool = false;
		bool right_bool = false;

		if (left.TryAs<runtime::Bool>() && right.TryAs<runtime::Bool>()) {
			if (left_bool = left.TryAs<runtime::Bool>()->GetValue() == true) {
				right_bool = right.TryAs<runtime::Bool>()->GetValue();
				return right;
			}
			return left;
		}
		else {
			throw std::runtime_error("Not implemented"s);
		}
	}

	ObjectHolder Not::Execute(Closure& closure, Context& context) {
		// Заглушка. Реализуйте метод самостоятельно
		return ObjectHolder::Own(runtime::Bool(!GetArg().get()->Execute(closure, context).TryAs<runtime::Bool>()->GetValue()));
	}

	Comparison::Comparison(Comparator /*cmp*/, unique_ptr<Statement> lhs, unique_ptr<Statement> rhs)
		: BinaryOperation(std::move(lhs), std::move(rhs)) {
		// Реализуйте метод самостоятельно
	}

	ObjectHolder Comparison::Execute(Closure& /*closure*/, Context& /*context*/) {
		// Заглушка. Реализуйте метод самостоятельно
		return {};
	}

	NewInstance::NewInstance(const runtime::Class& class_, std::vector<std::unique_ptr<Statement>> args) :cls_(class_) {
		// Заглушка. Реализуйте метод самостоятельно


	}

	NewInstance::NewInstance(const runtime::Class& class_) :cls_(class_) {
		// Заглушка. Реализуйте метод самостоятельно


	}

	ObjectHolder NewInstance::Execute(Closure& closure, Context& context) {
		// Заглушка. Реализуйте метод самостоятельно
		return ObjectHolder().Own(runtime::ClassInstance(cls_));
		//return {};
	}

	MethodBody::MethodBody(std::unique_ptr<Statement>&& /*body*/) {
	}

	ObjectHolder MethodBody::Execute(Closure& /*closure*/, Context& /*context*/) {
		// Заглушка. Реализуйте метод самостоятельно
		return {};
	}

}  // namespace ast