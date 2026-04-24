// RUN: %empty-directory(%t)
// RUN: split-file %s %t

// RUN: not %target-swift-frontend-verify -verify-additional-prefix a- -verify-additional-prefix b- -typecheck %t/test.swift 2>&1 | %update-verify-tests --prefix a- --prefix b-
// RUN: %target-swift-frontend-verify -verify-additional-prefix a- -verify-additional-prefix b- -typecheck %t/test.swift
// RUN: %diff %t/test.swift %t/test.swift.expected

//--- test.swift
// Each prefix has its own diagnostic on its own line.
func separateLinesPerPrefix() {
  unboundAlpha = 1 // expected-a-error{{wrong A}}
  unboundBeta = 1 // expected-b-error{{wrong B}}
}

// Whitespace preserved when a prefixed directive's message is rewritten.
func whitespacePreserved() {
  //  expected-a-error  @+1  {{wrong ws}}
  unboundDelta = 1
}

// Explicit count of 1 preserved through a message change for a prefixed directive.
func explicitOnePreserved() {
  // expected-b-error@+1 1{{wrong explicit}}
  unboundEpsilon = 1
}

// Unprefixed and prefixed directives, each matching a distinct actual.
func unprefixedAlongsidePrefixed() {
  unboundZeta = 1 // expected-error{{wrong universal}}
  unboundEta = 1 // expected-a-error{{wrong A only}}
}

// re1 path: count drops from 2 to 1 for a prefixed directive.
func countDropPerPrefix() {
  unboundLambda = 1 // expected-a-error 2{{wrong count}}
}

// count > 1 with wrong message: two actuals, count=2 expected, message wrong.
// Tightens text while preserving the explicit count.
func countTwoWithWrongMessage() {
  unboundMu = 1; unboundMu = 1 // expected-b-error 2{{wrong count msg}}
}

//--- test.swift.expected
// Each prefix has its own diagnostic on its own line.
func separateLinesPerPrefix() {
  unboundAlpha = 1 // expected-a-error{{cannot find 'unboundAlpha' in scope}}
  unboundBeta = 1 // expected-b-error{{cannot find 'unboundBeta' in scope}}
}

// Whitespace preserved when a prefixed directive's message is rewritten.
func whitespacePreserved() {
  //  expected-a-error  @+1  {{cannot find 'unboundDelta' in scope}}
  unboundDelta = 1
}

// Explicit count of 1 preserved through a message change for a prefixed directive.
func explicitOnePreserved() {
  // expected-b-error@+1 1{{cannot find 'unboundEpsilon' in scope}}
  unboundEpsilon = 1
}

// Unprefixed and prefixed directives, each matching a distinct actual.
func unprefixedAlongsidePrefixed() {
  unboundZeta = 1 // expected-error{{cannot find 'unboundZeta' in scope}}
  unboundEta = 1 // expected-a-error{{cannot find 'unboundEta' in scope}}
}

// re1 path: count drops from 2 to 1 for a prefixed directive.
func countDropPerPrefix() {
  unboundLambda = 1 // expected-a-error {{cannot find 'unboundLambda' in scope}}
}

// count > 1 with wrong message: two actuals, count=2 expected, message wrong.
// Tightens text while preserving the explicit count.
func countTwoWithWrongMessage() {
  unboundMu = 1; unboundMu = 1 // expected-b-error 2{{cannot find 'unboundMu' in scope}}
}

