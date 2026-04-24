// RUN: %target-swift-frontend -strict-concurrency=targeted -emit-sil -o /dev/null %s -verify
// RUN: %target-swift-frontend -strict-concurrency=complete -verify-additional-prefix complete- -emit-sil -o /dev/null %s -verify

// REQUIRES: concurrency
// REQUIRES: OS=macosx

@preconcurrency func send(_: Sendable) { }
func sendOpt(_: Sendable?) { }

enum E {
  case something(Sendable)
}

@available(SwiftStdlib 5.1, *)
func testE(a: Any, aOpt: Any?) async {
  send(a) // expected-warning{{type 'Any' does not conform to the 'Sendable' protocol}}
  sendOpt(a) // expected-warning{{type 'Any' does not conform to the 'Sendable' protocol; this is an error in the Swift 6 language mode}}
  sendOpt(aOpt) // expected-warning{{type 'Any' does not conform to the 'Sendable' protocol; this is an error in the Swift 6 language mode}}

  let _: E = .something(a) // expected-warning{{type 'Any' does not conform to the 'Sendable' protocol; this is an error in the Swift 6 language mode}}
  _ = E.something(a) // expected-warning{{type 'Any' does not conform to the 'Sendable' protocol; this is an error in the Swift 6 language mode}}

  var sendable: Sendable
  sendable = a // expected-warning{{type 'Any' does not conform to the 'Sendable' protocol; this is an error in the Swift 6 language mode}}

  var arrayOfSendable: [Sendable]
  arrayOfSendable = [a, a] // expected-warning 2{{type 'Any' does not conform to the 'Sendable' protocol; this is an error in the Swift 6 language mode}}

  func localFunc() { }
  sendable = localFunc // expected-warning{{type '() -> ()' does not conform to the 'Sendable' protocol; this is an error in the Swift 6 language mode}}
  // expected-note@-1{{a function type must be marked '@Sendable' to conform to 'Sendable'}}

  _ = sendable
  _ = arrayOfSendable
}

func testESilently(a: Any, aOpt: Any?) {
  send(a) // expected-complete-warning {{'Any' does not conform to the 'Sendable' protocol}}
  sendOpt(a) // expected-complete-warning {{type 'Any' does not conform to the 'Sendable' protocol; this is an error in the Swift 6 language mode}}
  sendOpt(aOpt) // expected-complete-warning {{type 'Any' does not conform to the 'Sendable' protocol; this is an error in the Swift 6 language mode}}

  let _: E = .something(a) // expected-complete-warning {{type 'Any' does not conform to the 'Sendable' protocol; this is an error in the Swift 6 language mode}}
  _ = E.something(a) // expected-complete-warning {{type 'Any' does not conform to the 'Sendable' protocol; this is an error in the Swift 6 language mode}}

  var sendable: Sendable
  sendable = a // expected-complete-warning {{type 'Any' does not conform to the 'Sendable' protocol; this is an error in the Swift 6 language mode}}

  var arrayOfSendable: [Sendable]
  arrayOfSendable = [a, a] // expected-complete-warning 2{{type 'Any' does not conform to the 'Sendable' protocol; this is an error in the Swift 6 language mode}}

  func localFunc() { }
  sendable = localFunc // expected-complete-warning {{type '() -> ()' does not conform to the 'Sendable' protocol; this is an error in the Swift 6 language mode}}
  // expected-complete-note @-1 {{a function type must be marked '@Sendable' to conform to 'Sendable'}}
  _ = sendable
  _ = arrayOfSendable
}

func testErasure() {
  class A {}
  class B : A {}

  func produce() -> any B & Sendable {
    fatalError()
  }

  let _: any A & Sendable = produce() // no warning
}
