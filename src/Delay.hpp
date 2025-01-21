#pragma once

#include <stdint.h>

uint32_t CalculateNextInterval(int stepsPerRotation, uint32_t currentSpeed, int32_t acceleration);