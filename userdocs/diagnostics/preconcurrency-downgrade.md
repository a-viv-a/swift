# Concurrency diagnostic downgraded due to `@preconcurrency` (PreconcurrencyDowngrade)

## Overview

`@preconcurrency` serves to ease adoption of Swift Concurrency and downgrades concurrency diagnostics to a lower severity. You can write `@preconcurrency` on your declarations and imports, and it can also be applied implicitly by the compiler. If you didn't write it, it may have been added by a library you use to preserve compatibility, or inferred based on the kind of module you imported. Diagnostics downgraded by `@preconcurrency` still represent concurrency issues; you should work to address them when possible even though your code compiles.

## Implicit `@preconcurrency`

These cases are implicitly `@preconcurrency`, meaning the attribute does not appear in code:

- Declarations imported from Clang modules (like C or Objective C) are implicitly `@preconcurrency`
- A closure passed to a `@preconcurrency` function is itself treated as `@preconcurrency`
- Declarations in a module built with `-default-isolation MainActor` are implicitly `@preconcurrency`

## Explicit `@preconcurrency`

- Declarations with the `@preconcurrency` attribute will downgrade diagnostics for uses of that declaration:

  ```swift
  @preconcurrency func doWork(_ callback: @Sendable () -> Void) { ... }

  var x = 0
  doWork {
    x += 1 // warning: main actor-isolated var 'x' can not be mutated from a nonisolated context
           // note: downgraded to a warning by '@preconcurrency' [#PreconcurrencyDowngrade]
  }
  ```

  This can be useful for adding concurrency annotations without breaking consumers that haven't adopted Swift Concurrency. It also avoids changing the mangling of `doWork` to preserve its ABI.

- A `@preconcurrency import` will downgrade diagnostics for uses of imported declarations:

  ```swift
  @preconcurrency import MyLibrary

  func test(ns: NonSendable) async {
    await sendToMainActor(ns) // warning: sending 'ns' risks causing data races
                              // note: downgraded to a warning by '@preconcurrency'
  }
  ```

  Applying the `@preconcurrency` attribute to an `import` allows importing a module that hasn't been updated with concurrency annotations while using strict concurrency checking. It reduces the reported severity of concurrency diagnostics caused by types from that module.

- A `@preconcurrency` protocol declaration downgrades `Sendable` member diagnostics for all conforming types:

  ```swift
  @preconcurrency protocol DataProcessor: Sendable {
    func process()
  }

  class MyProcessor: DataProcessor {
    var name: String // warning: stored property 'name' of 'Sendable'-conforming class 'MyProcessor' is mutable
                     // note: downgraded to a warning by '@preconcurrency'
    func process() { ... }
  }
  ```

  Like declaring a function `@preconcurrency`, this allows adding concurrency annotations without breaking implementations that existed before the annotation and aren't `Sendable` yet.

## See Also

- [Preconcurrency Attribute](https://docs.swift.org/swift-book/documentation/the-swift-programming-language/attributes/#preconcurrency)
- [Protocol conformances crossing into actor-isolated code (ConformanceIsolation)](conformance-isolation.md)
- [Sendable metatypes (SendableMetatypes)](sendable-metatypes.md)
