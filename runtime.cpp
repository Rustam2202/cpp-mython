#include "runtime.h"

#include <algorithm>
#include <cassert>
#include <optional>
#include <iostream> //
#include <sstream>

using namespace std;

namespace runtime {

	ObjectHolder::ObjectHolder(std::shared_ptr<Object> data)
		: data_(std::move(data)) {
	}

	void ObjectHolder::AssertIsValid() const {
		assert(data_ != nullptr);
	}

	ObjectHolder ObjectHolder::Share(Object& object) {
		// Возвращаем невладеющий shared_ptr (его deleter ничего не делает)
		return ObjectHolder(std::shared_ptr<Object>(&object, [](auto* /*p*/) { /* do nothing */ }));
	}

	ObjectHolder ObjectHolder::None() {
		return ObjectHolder();
	}

	Object& ObjectHolder::operator*() const {
		AssertIsValid();
		return *Get();
	}

	Object* ObjectHolder::operator->() const {
		AssertIsValid();
		return Get();
	}

	Object* ObjectHolder::Get() const {
		return data_.get();
	}

	ObjectHolder::operator bool() const {
		return Get() != nullptr;
	}

	bool IsTrue(const ObjectHolder& object) {
		if (object.TryAs<runtime::Number>()) {
			return object.TryAs<runtime::Number>()->GetValue();
		}
		else if (object.TryAs<runtime::Bool>()) {
			return object.TryAs<runtime::Bool>()->GetValue();
		}
		else if (object.TryAs<runtime::String>()) {
			return !object.TryAs<runtime::String>()->GetValue().empty();
		}
		else if (object.TryAs<runtime::Class>() || object.TryAs<runtime::ClassInstance>()) {
			return false;
		}
		return false;
	}

	void ClassInstance::Print(std::ostream& os, Context& context) {
		if (HasMethod("__str__"s, 0)) {
			os << Call("__str__"s, {}, context).TryAs<String>()->GetValue();
		}
		else {
			os << this;
		}
	}

	bool ClassInstance::HasMethod(const std::string& method, size_t argument_count) const {
		auto finded = class_.GetMethod(method);
		if (finded != nullptr) {
			if (finded->formal_params.size() == argument_count) {
				return true;
			}
		}
		return false;
	}

	Closure& ClassInstance::Fields() {
		return	closure_;
		throw std::logic_error("Not implemented"s);
	}

	const Closure& ClassInstance::Fields() const {
		return	closure_;
		throw std::logic_error("Not implemented"s);
	}

	ClassInstance::ClassInstance(const Class& cls) :class_(cls) {}

	ObjectHolder ClassInstance::Call(const std::string& method, const std::vector<ObjectHolder>& actual_args, Context& context) {
		Closure closure;
		closure["self"] = ObjectHolder::Share(*this);
		auto finded = class_.GetMethod(method);
		if (HasMethod(method, actual_args.size()) && finded != nullptr) {
			for (size_t i = 0; i < actual_args.size(); ++i) {
				closure[finded->formal_params[i]] = actual_args[i];
			}
			return (*finded).body.get()->Execute(closure, context);
		}
		else {
			throw std::runtime_error("Not implemented"s);
		}
	}

	Class::Class(std::string name, std::vector<Method> methods, const Class* parent) {
		parent_class_ = const_cast<Class*>(parent);
		methods_ = std::move(methods);
		if (parent_class_ == nullptr) {
			class_name_ = name;

		}
		else {
			class_name_ = parent->class_name_;
		}
		closure_["self"];
		for (const auto& method : methods_) {
			for (const auto& arg : method.formal_params) {
				closure_[arg];
			}
		}
	}

	const Method* Class::GetMethod(const std::string& name) const {
		auto finded = std::find_if(methods_.begin(), methods_.end(), [name](const Method& method) {return method.name == name; });
		if (finded != methods_.end()) {
			return &(*finded);
		}
		else if (parent_class_ != nullptr) {
			return parent_class_->GetMethod(name);
		}
		else {
			return nullptr;
		}
	}

	[[nodiscard]] const std::string& Class::GetName() const {
		return class_name_;
		//throw std::runtime_error("Not implemented"s);
	}

	void Class::Print(ostream& os, Context& /*context*/) {
		os << "Class "s << class_name_;
	}

	void Bool::Print(std::ostream& os, [[maybe_unused]] Context& context) {
		os << (GetValue() ? "True"sv : "False"sv);
	}

	bool Equal(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
		if (lhs.TryAs<Number>() && rhs.TryAs<Number>()) {
			return lhs.TryAs<Number>()->GetValue() == rhs.TryAs<Number>()->GetValue();
		}
		else if (lhs.TryAs<String>() && rhs.TryAs<String>()) {
			return lhs.TryAs<String>()->GetValue() == rhs.TryAs<String>()->GetValue();
		}
		else if (lhs.TryAs<Bool>() && rhs.TryAs<Bool>()) {
			return lhs.TryAs<Bool>()->GetValue() == rhs.TryAs<Bool>()->GetValue();
		}
		else if (lhs.Get() == nullptr && rhs.Get() == nullptr) {
			return true;
		}
		else if (lhs.TryAs<ClassInstance>()) {
			if (lhs.TryAs<ClassInstance>()->HasMethod("__eq__"s, 1)) {
				return lhs.TryAs<ClassInstance>()->Call("__eq__"s, { rhs }, context).TryAs<Bool>()->GetValue();
			}
		}
		throw std::runtime_error("Cannot compare objects for equality"s);
	}

	bool Less(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
		if (lhs.TryAs<Number>() && rhs.TryAs<Number>()) {
			return lhs.TryAs<Number>()->GetValue() < rhs.TryAs<Number>()->GetValue();
		}
		else if (lhs.TryAs<String>() && rhs.TryAs<String>()) {
			return lhs.TryAs<String>()->GetValue() < rhs.TryAs<String>()->GetValue();
		}
		else if (lhs.TryAs<Bool>() && rhs.TryAs<Bool>()) {
			return lhs.TryAs<Bool>()->GetValue() < rhs.TryAs<Bool>()->GetValue();
		}
		else if (lhs.TryAs<ClassInstance>()) {
			if (lhs.TryAs<ClassInstance>()->HasMethod("__lt__"s, 1)) {
				return lhs.TryAs<ClassInstance>()->Call("__lt__"s, { rhs }, context).TryAs<Bool>()->GetValue();
			}
		}

		throw std::runtime_error("Cannot compare objects for less"s);
	}

	bool NotEqual(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
		return !Equal(lhs, rhs, context);
		//	throw std::runtime_error("Cannot compare objects for equality"s);
	}

	bool Greater(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
		return !Less(lhs, rhs, context) && !Equal(lhs, rhs, context);
		//throw std::runtime_error("Cannot compare objects for equality"s);
	}

	bool LessOrEqual(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
		return !Greater(lhs, rhs, context);
		//throw std::runtime_error("Cannot compare objects for equality"s);
	}

	bool GreaterOrEqual(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
		return !Less(lhs, rhs, context);
		//throw std::runtime_error("Cannot compare objects for equality"s);
	}

}  // namespace runtime