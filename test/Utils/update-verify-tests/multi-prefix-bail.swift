// RUN: %empty-directory(%t)
// RUN: split-file %s %t

// RUN: not %target-swift-frontend-verify -verify-additional-prefix a- -verify-additional-prefix b- -typecheck %t/test.swift 2>%t/output.txt
// RUN: not %update-verify-tests --prefix a- --prefix b- < %t/output.txt | %FileCheck %s

// CHECK: cannot synthesize new directive for unexpected diagnostic in a multi-prefix file

//--- test.swift
let satisfiedExpectation = 1 // expected-warning {{initialization of immutable value 'satisfiedExpectation' was never used; consider replacing with assignment to '_' or removing it}}
unboundUnexpected = 1
