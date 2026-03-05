// include/logic_element.h - Abstract base class for LD logic elements
#pragma once

#include <memory>
#include <string>
#include "plcio.h"

class LogicElement {
public:
    virtual bool evaluate(std::shared_ptr<PlcIO> io) = 0;
    virtual std::string toString() const = 0;
    virtual ~LogicElement() = default;
};
