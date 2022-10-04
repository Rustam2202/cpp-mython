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
		closure[name_] = rv_.get()->Execute(closure, context);
		return closure.at(name_);
	}

	Assignment::Assignment(std::string var, std::unique_ptr<Statement> rv) {
		name_ = var;
		rv_ = std::move(rv);
	}

	VariableValue::VariableValue(const std::string& var_name) {
		name_ = var_name;
		//dotted_ids_.push_back(var_name);
	}

	VariableValue::VariableValue(std::vector<std::string> dotted_ids)
		//:dotted_ids_(dotted_ids) 
	{
		if (auto size = dotted_ids.size(); size > 0) {
			name_ = std::move(dotted_ids.at(0));
			dotted_ids_.resize(size - 1);
			std::move(std::next(dotted_ids.begin()), dotted_ids.end(), dotted_ids_.begin());
		}

		/*if (dotted_ids.size() > 0) {
			name_ = dotted_ids.front();
			dotted_ids_ = { dotted_ids.begin() + 1,dotted_ids.end() };
		}*/
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
		if (cls /*rv_*/) {
			return cls->Fields()[field_name_] = rv_.get()->Execute(closure, context);
		}
		else {
			throw runtime_error("Object is not class"s);
		}
		//return cls->Fields().at(field_name_);
	}

	ObjectHolder VariableValue::Execute(Closure& closure, Context& context) {
		//using namespace runtime;

		//if (dotted_ids_.size() > 0)
		//{
		//	runtime::ObjectHolder result;
		//	Closure* current_closure_ptr = &closure;
		//	for (const auto& arg : dotted_ids_)
		//	{
		//		auto arg_it = current_closure_ptr->find(arg);
		//		if (arg_it == current_closure_ptr->end())
		//		{
		//			throw std::runtime_error("Invalid argument name in VariableValue::Execute()"s);
		//		}
		//		result = arg_it->second;
		//		auto next_dotted_arg_ptr = result.TryAs<runtime::ClassInstance>();
		//		if (next_dotted_arg_ptr)
		//		{
		//			current_closure_ptr = &next_dotted_arg_ptr->Fields();
		//		}
		//	}
		//	return result;
		//}
		// Во всех других случаях выбрасываем исключение
		//throw std::runtime_error("No arguments specified for VariableValue::Execute()"s);

		if (closure.count(name_)) {
			auto result = closure.at(name_);
			if (dotted_ids_.size() > 0) {
				if (auto obj = result.TryAs<runtime::ClassInstance>()) {
					return VariableValue(dotted_ids_).Execute(obj->Fields(), context);
				}
				else {
					throw std::runtime_error("Variable " + name_ + " is not class"s);
				}
			}
			return result;
		}
		else {
			throw std::runtime_error("Variable "s + name_ + " not found"s);
		}

		return {};
	}

	unique_ptr<Print> Print::Variable(const std::string& name) {
		return make_unique<Print>(Print{ make_unique<VariableValue>(name) });
	}

	Print::Print(unique_ptr<Statement> argument) {
		args_.push_back(std::move(argument));
	}

	Print::Print(vector<unique_ptr<Statement>> args) :args_(std::move(args)) {	}


	ObjectHolder Print::Execute(Closure& closure, Context& context) {
		// Заглушка. Реализуйте метод самостоятельно
		auto it = args_.begin();
		ObjectHolder result;

		while (true) {
			result = (*it).get()->Execute(closure, context);
			if (result) {
				result.Get()->Print(context.GetOutputStream(), context);
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

		return ObjectHolder::None();
	}

	//MethodCall::MethodCall(std::unique_ptr<Statement> object, std::string method, std::vector<std::unique_ptr<Statement>> args) {
	//	// Заглушка. Реализуйте метод самостоятельно

	//}

	ObjectHolder MethodCall::Execute(Closure& closure, Context& context) {
		// Заглушка. Реализуйте метод самостоятельно
		auto cls = object_->Execute(closure, context).TryAs<runtime::ClassInstance>();
		if (cls) {
			std::vector<runtime::ObjectHolder> actual_args;
			for (auto& arg : args_) {
				actual_args.push_back(arg->Execute(closure, context));
			}
			return cls->Call(method_, actual_args, context);
		}
		else {
			throw std::runtime_error("Object is not class instance"s);
		}
	}

	ObjectHolder Stringify::Execute(Closure& closure, Context& context) {
		std::string str;
		ObjectHolder value = GetArg().get()->Execute(closure, context);
		if (value) {
			std::ostringstream os;
			value.Get()->Print(os, context);
			return ObjectHolder::Own(runtime::String(os.str()));
		}
		else {
			return ObjectHolder::Own(runtime::String("None"s));
		}

		/*if (value.TryAs<runtime::Number>()) {
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
		return ObjectHolder::Own(runtime::String(str));*/
	}

	ObjectHolder Add::Execute(Closure& closure, Context& context) {
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
		for (auto& arg : args_) {
			arg->Execute(closure, context);
		}
		return ObjectHolder::None();
	}

	ObjectHolder Return::Execute(Closure& closure, Context& context) {
		throw statement_.get()->Execute(closure, context);
	}

	ClassDefinition::ClassDefinition(ObjectHolder cls) :class_(std::move(cls)) {	}

	ObjectHolder ClassDefinition::Execute(Closure& closure, Context& context) {
		// Заглушка. Реализуйте метод самостоятельно
		closure[class_.TryAs<runtime::Class>()->GetName()] = class_;
		return ObjectHolder::None();
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

	NewInstance::NewInstance(const runtime::Class& class_, std::vector<std::unique_ptr<Statement>> args) :cls_(class_) {	}

	NewInstance::NewInstance(const runtime::Class& class_) :cls_(class_) {	}

	ObjectHolder NewInstance::Execute(Closure& closure, Context& context) {
		//return ObjectHolder().Own(runtime::ClassInstance(cls_));

		if (cls_.HasMethod(INIT_METHOD, args_.size())) {
			std::vector<runtime::ObjectHolder> actual_args;
			for (auto& arg : args_) {
				actual_args.push_back(arg->Execute(closure, context));
			}
			cls_.Call(INIT_METHOD, actual_args, context);
		}
		return runtime::ObjectHolder::Share(cls_);
	}

	//MethodBody::MethodBody(std::unique_ptr<Statement>&& /*body*/) {	}

	ObjectHolder MethodBody::Execute(Closure& closure, Context& context) {
		// Заглушка. Реализуйте метод самостоятельно
		try {
			body_->Execute(closure, context);
			return runtime::ObjectHolder::None();
		}
		catch (runtime::ObjectHolder& result) {
			return result;
		}
		catch (...) {
			throw;
		}
	}

}  // namespace ast