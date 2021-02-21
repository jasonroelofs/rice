#include "Data_Object.hpp"
#include "String.hpp"

template<typename Enum_T>
inline Rice::Enum_Storage<Enum_T>::
Enum_Storage(std::string name, Enum_T value) : enumName(name), enumValue(value)
{
}

template<typename Enum_T>
inline bool Rice::Enum_Storage<Enum_T>::
operator==(const Enum_Storage<Enum_T>& other)
{
  return this->enumValue == other.enumValue;
}

template<typename Enum_T>
inline int32_t Rice::Enum_Storage<Enum_T>::
compare(const Rice::Enum_Storage<Enum_T>& other)
{
  if (this->enumValue == other.enumValue)
  {
    return 0;
  }
  else if (this->enumValue < other.enumValue)
  {
    return -1;
  }
  else
  {
    return 1;
  }
}

template<typename Enum_T>
Rice::Enum<Enum_T>::
Enum(
    char const * name,
    Module module)
  : Data_Type<Enum_Storage<Enum_T>>()
{
  this->bind(initialize(name, module));
}

template<typename Enum_T>
Rice::Enum<Enum_T> & Rice::Enum<Enum_T>::
initialize(
    char const * name,
    Module module)
{
  Class c = Rice::define_class_under<Enum_T>(module, name)
    .define_method("to_s", to_s)
    .define_method("to_i", to_i)
    .define_method("inspect", inspect)
    .define_method("<=>", compare)
    .define_method("hash", hash)
    .define_method("eql?", eql)
    .define_method("==", eql)
    .define_method("===", eql)
    .define_singleton_method("each", each)
    .define_singleton_method("from_int", from_int)
    .include_module(rb_mComparable);

  // Create a Ruby array that we will use later to store enum values
  // and attach it to the class
  Array enums;
  protect(rb_iv_set, c, "enums", enums);

  // Since we just changed the underlying class we need to rebind this enum
  // before returning. Otherwise this->value() will bind the wrong class to 
  // the datatype
  this->set_value(c);
  return *this;
}

template<typename Enum_T>
Rice::Enum<Enum_T> & Rice::Enum<Enum_T>::
define_value(
    std::string name,
    Enum_T value)
{
  // Create a new storage struct for this value
  Storage_T* storage = new Storage_T(name, value);

  // Now wrap it and store it to the class enums field
  Value_T wrapper(storage);
  Array enums = rb_iv_get(this->value(), "enums");
  enums.push(wrapper);

  // And store it as class constant
  this->const_set(name, wrapper);

  return *this;
}

template<typename Enum_T>
Rice::Object Rice::Enum<Enum_T>::
each(Object self)
{
  Array enums = rb_iv_get(self, "enums");
  Check_Type(enums, T_ARRAY);
  for(long i = 0; i < enums.size(); ++i)
  {
    rb_yield(enums[i].value());
  }
  return Qnil;
}

template<typename Enum_T>
Rice::Object Rice::Enum<Enum_T>::
to_s(Object self)
{
  Value_T wrapper(self);
  return detail::to_ruby(wrapper->enumName);
}

template<typename Enum_T>
Rice::Object Rice::Enum<Enum_T>::
inspect(Object self)
{
  Value_T wrapper(self);

  return String::format(
      "#<%s::%s>",
      String(self.class_of().name()).c_str(),
      wrapper->enumName.c_str());
}

template<typename Enum_T>
Rice::Object Rice::Enum<Enum_T>::
compare(Object lhs, Object rhs)
{
  if(lhs.class_of() != rhs.class_of())
  {
    String lhs_name(lhs.class_of().name());
    String rhs_name(rhs.class_of().name());
    rb_raise(
        rb_eTypeError,
        "Cannot compare %s to %s",
        lhs_name.c_str(),
        rhs_name.c_str());
  }

  Value_T left(lhs);
  Value_T right(rhs);

  int32_t result = left->compare(*right);
  return detail::To_Ruby<int>::convert(result);
}

template<typename Enum_T>
Rice::Object Rice::Enum<Enum_T>::
eql(Object lhs, Object rhs)
{
  Value_T left(lhs);
  Value_T right(rhs);
  bool is_equal = (*left == *right);
  return detail::To_Ruby<bool>::convert(is_equal);
}

template<typename Enum_T>
Rice::Object Rice::Enum<Enum_T>::
to_i(Object self)
{
  using Integral_T = std::underlying_type_t<Enum_T>;
  Value_T wrapper(self);
  return detail::To_Ruby<Integral_T>::convert((Integral_T)wrapper->enumValue);
}

template<typename Enum_T>
Rice::Object Rice::Enum<Enum_T>::
hash(Object self)
{
  return to_i(self);
}

template<typename Enum_T>
Rice::Object Rice::Enum<Enum_T>::
from_int(Class klass, Object i)
{
  Enum_T enumValue = static_cast<Enum_T>(detail::From_Ruby<long>::convert(i));
  Array enums = rb_iv_get(klass, "enums");

  auto iter = std::find_if(enums.begin(), enums.end(),
    [enumValue](const Object& object)
    {
      Value_T dataObject(object);
      return dataObject->enumValue == enumValue;
    });

  if (iter == enums.end())
  {
    rb_raise(
      rb_eArgError,
      "Invalid Enum value %i", (int)enumValue);
  }

  return *iter;
}

template<typename T>
Rice::Enum<T> Rice::
define_enum(
    char const * name,
    Module module)
{
  return Enum<T>(name, module);
}