# Performance Design Choices (性能设计决策)

This document tracks performance-critical architectural decisions made during NVRHI-Lab development. Each component is analyzed for optimization opportunities and documented with rationale.

## Core Principle (核心原则)

**Measure before optimize.** Most "optimizations" have negligible impact. Focus on GPU work, not CPU dispatch.

## General Guidelines (通用指南)

### When to Optimize (何时优化)

✓ **Do optimize:**
- GPU command submission
- Memory allocations in hot paths
- State changes (pipelines, shaders, resources)
- Draw call batching
- Synchronization points

✗ **Don't optimize:**
- Architecture choices (virtual vs templates vs switch)
- One-time initialization
- Code called <1000 times per frame
- Premature optimization without profiling

### Optimization Workflow (优化流程)

1. **Profile first** - Use tools like NSight, PIX, Tracy
2. **Identify bottleneck** - CPU vs GPU bound
3. **Target top 10%** - Fix issues that matter most
4. **Measure impact** - Verify with before/after benchmarks
5. **Document decision** - Add to this file

### Common Pitfalls (常见陷阱)

- **Premature optimization** - Optimizing without profiling
- **Micro-optimizations** - Saving 10 cycles in a 16ms frame
- **Architectural over-engineering** - Complex designs for negligible gains
- **Ignoring GPU** - CPU-bound optimizations when GPU is the limit

---

## Key Component 1: DeviceManager (设备管理器)

### Architectural Options Considered

1. **Runtime Switching** (void* + switch statements)
2. **Polymorphism** (virtual functions)
3. **Templates** (compile-time dispatch)

### Decision

**Chosen: Runtime Switching**

### Rationale

| Factor | Impact | Notes |
|---------|----------|--------|
| Performance | <0.001% | Called once per frame, overhead negligible |
| Simplicity | High | Single class, all code in one place |
| Maintainability | High | Easy to understand and modify |
| Flexibility | Runtime | Switch backends at runtime via config |

### Quantitative Analysis

60 FPS application:
- Frame time: 16.67ms
- DeviceManager overhead: ~0.000016ms (50 cycles)
- Percentage: 0.0001%

**Conclusion:** Architectural choice irrelevant to frame rate. Bottleneck is GPU work.

## References

- "Premature optimization is the root of all evil" - Donald Knuth
- [NVRHI Programming Guide](../thirdparty/NVRHI/doc/ProgrammingGuide.md)
- [NVRHI Tutorial](../thirdparty/NVRHI/doc/Tutorial.md)