#include "Configuration/ConfigurationType.h"

namespace AdvancedWebserver {

ConfigurationType::ConfigurationType(const std::string &_type,
                                     const std::string &_description)
    : m_type(_type), m_description(_description) {}

// bool ConfigurationType::operator==(const ConfigurationType &_ct) {
//   if (_ct.GetType() == m_type && _ct.GetDescription() == m_description) {
//     return true;
//   } else {
//     return false;
//   }
// }

const std::string &ConfigurationType::GetType() const { return m_type; }
const std::string &ConfigurationType::GetDescription() const {
  return m_description;
}

void ConfigurationType::SetType(const std::string &_type) { m_type = _type; }
void ConfigurationType::SetDescription(const std::string &_description) {
  m_description = _description;
}

ConfigurationType::~ConfigurationType() {}
} // namespace AdvancedWebserver
