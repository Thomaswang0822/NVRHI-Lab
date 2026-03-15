# Version Log

## 0.1.0

A minimal but runnable app with wxWidgets GUI. 

- NVRHI has been added via CMake but not used in app.
- Design and implementation roadmap have been decided.

## 0.1.1

Phase 1, Task 1: Device Manager Implementation

- Created DeviceManager with multi-backend support (D3D11/D3D12/Vulkan stub)
- Added performance design documentation (docs/performance-design-choices.md)
- Documented architectural decision: Runtime switching vs polymorphism analysis
- Concluded: Architectural choice has negligible performance impact (<0.001%)
