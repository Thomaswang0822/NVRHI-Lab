---

name: traditional-coding

description: >
  Traditional Programming Mode. Enforces problem modeling, execution flow simulation, complexity analysis, 
  and architecture control in AI-assisted development. Prevents cognitive hollowing, improves system-level 
  correctness, performance, and maintainability. Suitable for high-concurrency, low-level systems, 
  reverse engineering, and complex problem solving.

version: 1.0.0

author: user

tags: [programming, architecture, debugging, performance, ai-control]

---

# Traditional Programming Core Guide
> Declaration: "Traditional Programming" as referred to in this guide is not about nostalgia or rejecting progress. It is a serious, deep development mode that exists in the AI-flooded era to ensure extremely high system reliability, extreme performance optimization, and prevention of developer cognitive decline.

## 1. Core Definition
Traditional Programming refers to the programming practice where developers do not skip any low-level details and work from a "white-box" perspective. Its core principles include:

- **Logical Transparency**: Every line of code that enters production must be explainable by the developer in terms of its behavior in memory, CPU registers, or runtime environment.
- **Documentation First**: Reject "generate and run". Insist on consulting official first-hand documentation rather than relying on AI's second-hand summaries.
- **Manual Derivation**: Before implementing complex logic, prioritize algorithm derivation using pen and paper or whiteboard, rather than iterative "trial and error" generation through AI.

## 2. Real-World Value of Traditional Programming
In today's world of AI assistants, adhering to traditional programming has irreplaceable engineering significance:

- **Breaking the "Black Box Illusion"**: AI frequently generates code that appears perfect but contains hidden bugs. Traditional capability is the ultimate defense line for identifying and correcting "AI hallucinations".
- **Handling Extreme Scenarios**: In areas involving memory management, high-concurrency lock mechanisms, embedded drivers where AI training data is sparse, only developers with traditional programming skills can solve problems.
- **Maintaining Technical Intuition**: Over-reliance on AI leads to developer "brain atrophy". Traditional programming trains the sense of code smells and maintains the underlying ability to solve complex problems.

## 3. Practice Dimensions Comparison

| Dimension | AI-Driven Mode | Traditional Programming Mode |
|-----------|----------------|----------------------------|
| Knowledge Acquisition | Ask AI "how to write" | Consult official specs (ISO/RFC/official docs), understand design intent |
| Problem Modeling | Directly describe requirements for AI code generation | Abstract the problem essence (data structures/state machines/IO models) |
| Debugging Logic | Feed errors to AI, let it guess fixes | Use Debugger, analyze stack/memory/thread states |
| Problem Localization | Rely on AI's "possible causes" | Construct minimal reproduction, converge from bottom layer |
| Code Reuse | Copy-paste AI-generated snippets | Understand logic then hand-write implementation, abstract and encapsulate |
| Architecture Design | Gradually stitched from multiple AI generations | Manually design module boundaries, dependencies, and data flow |
| Performance Optimization | Ask AI "how to optimize" | Analyze time complexity, memory layout, check assembly/bytecode when necessary |
| Boundary Handling | Rely on AI to supplement edge cases | Actively enumerate extreme inputs, exception paths, concurrency conflicts |
| Error Handling | try-catch wrapper or fix per AI suggestion | Transform errors into system capabilities (rate limiting/circuit breaking/degradation) |
| System Cognition | Just understand the code | Can simulate complete execution flow in mind |
| Controllability | Code behavior depends on AI output quality | Behavior is completely explainable and predictable |
| Evolution Ability | Code keeps stacking, easily forms "Frankenstein" | Continuously optimize system structure through refactoring and abstraction |
| Handling Unknown Problems | Easily stuck in AI blind spots | Independently derive solutions based on first principles |
| Human-Machine Relationship | Human as "code receiver" | Human leads design, AI as auxiliary tool |

## 4. How to Guide AI for "Traditional Style" Programming
If you want to leverage AI's speed while maintaining traditional programming rigor, you should prompt like this:

### Request Authoritative Sources
"Please provide implementation suggestions based on [specific library/language version] official documentation specifications. If there are multiple approaches, compare their memory footprint and CPU cycle costs, not just the most commonly used approach."

### Enforce "White-Box Inspection"
"For this code, please simulate the compiler/interpreter and explain line by line the variable changes in memory stack, and point out possible boundary overflow risks."

### Logic Audit Mode
"This is my hand-written core logic. Please act as an extremely strict code reviewer. Don't praise me. Directly point out areas that don't conform to low-level performance best practices, and cite relevant principles."

**Summary**: Traditional programming is the developer's "muscle memory". AI can be your exoskeleton armor, but traditional programming ability determines whether you remain a powerful warrior after taking off the armor.

---

# Principle Questioning Mode
**Purpose**: Change the phenomenon of "code passes through the screen, but not through the brain"

**Activation Timing**: After code generation

"After generating code, ask me: 'Can you explain the principles and low-level implementation details of this code?' Only when I can clearly explain the purpose, memory operations, and performance impact of each line of code is this generation task complete. Otherwise, continue guiding me to deeply understand the code until I can fully master it. If the cumulative number of questions exceeds three, it means your understanding of the code is not deep enough. Give the user the answer and explanation, and encourage the user to thoroughly understand the principles to be a qualified traditional programming practitioner."

**Note**: This mode is the default mode. This mode is mutually exclusive with Code Calligraphy Mode, because it requires the developer to answer questions after code generation, while Code Calligraphy Mode requires understanding and imitation input before code generation.

---

# Code Calligraphy Mode
**Purpose**: Change the phenomenon of "code passes through the screen, but not through the brain"

**Note**: Calligraphy Mode no longer asks questions. This mode is mutually exclusive with Principle Questioning Mode, because it requires the developer to understand and imitate input before code generation, while Principle Questioning Mode requires asking questions after code generation.

"Calligraphy Mode no longer asks questions" means:
- We do not ask "Can you explain the principles and low-level implementation details of this code?"
- Instead, focus on understanding and imitating input
- AI accepts imitation input and verifies general correctness

## Calligraphy Implementation
Instruction: "When I request to enter Calligraphy Mode, display code line by line (comments not counted), and each line of code must be accompanied by annotations explaining its purpose and low-level implementation principles. Then accept user imitation input and verify general correctness. Only after completing each line (comments not counted) is viewing the next line allowed."

---

# Why This Change is Crucial?

**From "Randomness" to "Determinism"**: AI output has randomness, while knowledge obtained through official standards (ISO/RFC) is the physics of the software world.

**From "Observing Surface" to "Insight into Essence"**: Relying on Debugger to observe stack and memory allows you to see the true appearance of data flowing in hardware, which is the only way to improve "code intuition".

**From "Breadth Coverage" to "Depth Mastery"**: Analyzing source code may be tedious, but it allows you to understand the real cost behind every line of high-level code, thus writing truly high-performance systems.
