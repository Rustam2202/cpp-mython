#include "runtime.h"

#include <algorithm>
#include <cassert>
#include <optional>
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
		// Заглушка. Реализуйте метод самостоятельно
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

	void ClassInstance::Print(std::ostream& /*os*/, Context& /*context*/) {
		// Заглушка, реализуйте метод самостоятельно
	}

	bool ClassInstance::HasMethod(const std::string& method, size_t argument_count) const {
		// Заглушка, реализуйте метод самостоятельно

		auto finded = class_.GetMethod(method);
		if (finded != nullptr) {
			if (finded->formal_params.size() == argument_count) {
				return true;
			}
		}
		return false;
	}

	Closure& ClassInstance::Fields() {
		// Заглушка. Реализуйте метод самостоятельно
		return	closure_;
		throw std::logic_error("Not implemented"s);
	}

	const Closure& ClassInstance::Fields() const {
		// Заглушка. Реализуйте метод самостоятельно
		return	closure_;
		throw std::logic_error("Not implemented"s);
	}

	ClassInstance::ClassInstance(const Class& cls)
		:class_(cls)
	{
		// Реализуйте метод самостоятельно

	}

	ObjectHolder ClassInstance::Call(const std::string& method, const std::vector<ObjectHolder>& actual_args, Context& context) {
		// Заглушка. Реализуйте метод самостоятельно.

		closure_["self"] = ObjectHolder::Share(*this);
		auto finded = class_.GetMethod(method);
		if (finded != nullptr) {
			//	return (*finded).body.get()->Execute
		}
		else {
			throw std::runtime_error("Not implemented"s);
		}
	}

	Class::Class(std::string name, std::vector<Method> methods, const Class* parent)
		//:class_name_(name),methods_(methods),parent_class_(parent)
	{
		// Реализуйте метод самостоятельно
		if (parent != nullptr) {
			class_name_ = parent->class_name_;
			//	methods_ = std::move(parent->methods_);
		}
		else {
			class_name_ = name;
			methods_ = std::move(methods);
		}
		closure_["self"];
		for (const auto& method : methods_) {
			for (const auto& arg : method.formal_params) {
				closure_[arg];
			}
		}
	}

	const Method* Class::GetMethod(const std::string& name) const {
		// Заглушка. Реализуйте метод самостоятельно

		auto finded = std::find_if(methods_.begin(), methods_.end(), [name](const Method& method) {return method.name == name; });
		if (finded != methods_.end()) {
			return &(*finded);
		}
		else {
			return nullptr;
		}
	}

	[[nodiscard]] /*inline*/ const std::string& Class::GetName() const {
		// Заглушка. Реализуйте метод самостоятельно.

		return class_name_;

		throw std::runtime_error("Not implemented"s);
	}

	void Class::Print(ostream& os, Context& /*context*/) {
		// Заглушка. Реализуйте метод самостоятельно

		os << "Class "s << class_name_;
	}

	void Bool::Print(std::ostream& os, [[maybe_unused]] Context& context) {
		os << (GetValue() ? "True"sv : "False"sv);
	}

	bool Equal(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
		// Заглушка. Реализуйте функцию самостоятельно
		if (lhs.TryAs<Number>() && rhs.TryAs<Number>()) {
			return lhs.TryAs<Number>()->GetValue() == rhs.TryAs<Number>()->GetValue();
		}
		else if (lhs.TryAs<String>() && rhs.TryAs<String>()) {
			return lhs.TryAs<String>()->GetValue() == rhs.TryAs<String>()->GetValue();
		}
		else if (lhs.TryAs<Bool>() && rhs.TryAs<Bool>()) {
			return lhs.TryAs<Bool>()->GetValue() == rhs.TryAs<Bool>()->GetValue();
		}

		throw std::runtime_error("Cannot compare objects for equality"s);
	}

	bool Less(const ObjectHolder& /*lhs*/, const ObjectHolder& /*rhs*/, Context& /*context*/) {
		// Заглушка. Реализуйте функцию самостоятельно
		throw std::runtime_error("Cannot compare objects for less"s);
	}

	bool NotEqual(const ObjectHolder& /*lhs*/, const ObjectHolder& /*rhs*/, Context& /*context*/) {
		// Заглушка. Реализуйте функцию самостоятельно
		throw std::runtime_error("Cannot compare objects for equality"s);
	}

	bool Greater(const ObjectHolder& /*lhs*/, const ObjectHolder& /*rhs*/, Context& /*context*/) {
		// Заглушка. Реализуйте функцию самостоятельно
		throw std::runtime_error("Cannot compare objects for equality"s);
	}

	bool LessOrEqual(const ObjectHolder& /*lhs*/, const ObjectHolder& /*rhs*/, Context& /*context*/) {
		// Заглушка. Реализуйте функцию самостоятельно
		throw std::runtime_error("Cannot compare objects for equality"s);
	}

	bool GreaterOrEqual(const ObjectHolder& /*lhs*/, const ObjectHolder& /*rhs*/, Context& /*context*/) {
		// Заглушка. Реализуйте функцию самостоятельно
		throw std::runtime_error("Cannot compare objects for equality"s);
	}

}  // namespace runtime