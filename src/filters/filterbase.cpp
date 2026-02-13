#include "../../include/filters/filterbase.h"
#include <string>

FilterBase::FilterBase(const std::string &name) : m_name(name) {}

std::string FilterBase::getName() const { return m_name; }
