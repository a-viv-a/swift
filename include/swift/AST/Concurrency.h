//===--- Concurrency.h ----------------------------------------------------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2024 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_AST_CONCURRENCY_H
#define SWIFT_AST_CONCURRENCY_H

#include "swift/AST/DiagnosticEngine.h"

namespace swift {

/// \c ConcurrencyDiagnosticBehavior is a wrapper around multiple optional
/// 'reasons' with associated data for downgrading concurrency diagnostics. It
/// allows us to record and ergonomically merge different motivations for
/// downgrading a concurrency related diagnostic.
///
/// \c ConcurrencyDiagnosticBehavior with all reason fields empty is like a \c
/// nullopt of \c std::optional<DiagnosticBehavior> but with the ability to
/// merge the 'empty' behaviors.
struct ConcurrencyDiagnosticBehavior {
  /// Combines a \c DiagnosticBehavior with a \c LanguageMode where it will no
  /// longer apply.
  ///
  /// \c until *must* be a language mode after the relevant mode for this code,
  /// or \c LanguageStaging will be merged into other
  /// \c ConcurrencyDiagnosticBehavior when it doesn't apply, and downgrade
  /// diagnostics incorrectly.
  struct LanguageStaging {
    /// The current diagnostic behavior.
    DiagnosticBehavior behavior;
    /// The future language version, where this behavior downgrade will not be in effect.
    LanguageMode until;
  };

private:
  std::optional<DiagnosticBehavior> preconcurrency;
  std::optional<LanguageStaging> languageStaging;

  /// Constructors go through the factories so the \c languageStaging invariant
  /// ("set only while staging is still active") is enforced.
  ConcurrencyDiagnosticBehavior(
      std::optional<DiagnosticBehavior> preconcurrency,
      std::optional<LanguageStaging> languageStaging)
      : preconcurrency(preconcurrency), languageStaging(languageStaging) {}

public:
  /// Construct the empty ('no preference') behavior.
  ConcurrencyDiagnosticBehavior() = default;

  const std::optional<DiagnosticBehavior> &getPreconcurrency() const {
    return preconcurrency;
  }
  const std::optional<LanguageStaging> &getLanguageStaging() const {
    return languageStaging;
  }

  [[nodiscard]] static ConcurrencyDiagnosticBehavior
  forPreconcurrency(DiagnosticBehavior behavior = DiagnosticBehavior::Warning) {
    return ConcurrencyDiagnosticBehavior(behavior, std::nullopt);
  }

  /// \returns an empty behavior when \p condition is false, to compose
  /// "conditionally downgrade via @preconcurrency" without a scope block or
  /// ternary.
  [[nodiscard]] static ConcurrencyDiagnosticBehavior forPreconcurrencyIf(
      bool condition,
      DiagnosticBehavior behavior = DiagnosticBehavior::Warning) {
    if (!condition)
      return {};
    return forPreconcurrency(behavior);
  }

  /// Construct a \c languageStaging reason iff staging is still active for \p
  /// currentVersion.
  ///
  /// \returns \c LanguageStaging based behavior if \p until is after \p
  /// currentVersion, otherwise an empty behavior.
  [[nodiscard]] static ConcurrencyDiagnosticBehavior
  forLanguageStaging(DiagnosticBehavior behavior, LanguageMode until,
                     version::Version currentVersion) {
    if (until.isEffectiveIn(currentVersion))
      return {};
    return ConcurrencyDiagnosticBehavior(std::nullopt,
                                         LanguageStaging{behavior, until});
  }

  /// Convenience overload that extracts the current language version from
  /// \p ctx.
  [[nodiscard]] static ConcurrencyDiagnosticBehavior
  forLanguageStaging(DiagnosticBehavior behavior, LanguageMode until,
                     const ASTContext &ctx);

  /// \returns an empty behavior when \p condition is false, to compose
  /// "conditionally downgrade via language staging" without a scope block or
  /// ternary.
  [[nodiscard]] static ConcurrencyDiagnosticBehavior
  forLanguageStagingIf(bool condition, DiagnosticBehavior behavior,
                       LanguageMode until,
                       version::Version currentVersion) {
    if (!condition)
      return {};
    return forLanguageStaging(behavior, until, currentVersion);
  }

  /// Convenience overload that extracts the current language version from
  /// \p ctx.
  [[nodiscard]] static ConcurrencyDiagnosticBehavior
  forLanguageStagingIf(bool condition, DiagnosticBehavior behavior,
                       LanguageMode until, const ASTContext &ctx);

  /// Lattice merge the reasons in \c this and \p other while accounting for
  /// empty fields, and return a new \c ConcurrencyDiagnosticBehavior.
  ///
  /// For multiple \c languageStaging fields, takes the latest \c LanguageMode
  /// since this means there are multiple language stagings restricting
  /// the diagnostic, and the furthest in the future one will 'outlive' the
  /// others.
  ///
  /// \returns the merged behavior.
  [[nodiscard]] ConcurrencyDiagnosticBehavior
  merge(ConcurrencyDiagnosticBehavior other) const;

  /// Merge all present \c DiagnosticBehavior fields, returning the max value
  /// if any were present. Using merge() and threading
  /// \c ConcurrencyDiagnosticBehavior down to where behaviors are used is
  /// preferable for preserving information about why a diagnostic is
  /// downgraded.
  ///
  /// \returns The lattice merge of all present \c BehaviorDiagnostics, or
  /// \c nullopt if none are present.
  std::optional<DiagnosticBehavior> merged() const {
    if (preconcurrency && languageStaging)
      return preconcurrency.value().merge(languageStaging.value().behavior);
    if (languageStaging)
      return languageStaging.value().behavior;
    return preconcurrency;
  }

  /// Check if any field asks the diagnostic to be ignored entirely.
  /// \returns true if any present field is \c DiagnosticBehavior::Ignore
  bool isIgnored() const {
    return merged().value_or(DiagnosticBehavior::Unspecified)
           == DiagnosticBehavior::Ignore;
  }
  /// Check if this behavior is effectively "no preference", because the only
  /// fields present are \c DiagnosticBehavior::Unspecified at most.
  // \returns true if no fields are present, or the present fields are \c
  // DiagnosticBehavior::Unspecified
  bool isUnspecified() const {
    return merged().value_or(DiagnosticBehavior::Unspecified)
           == DiagnosticBehavior::Unspecified;
  }

  explicit operator bool() const {
    return preconcurrency || languageStaging;
  }
};

/// Find the imported module that treats the given nominal type as "preconcurrency", or return `nullptr`
/// if there is no such module.
ModuleDecl *moduleImportForPreconcurrency(NominalTypeDecl *nominal,
                                          const DeclContext *fromDC);

/// Determinate the appropriate diagnostic behavior to used when emitting
/// concurrency diagnostics when referencing the given nominal type from the
/// given declaration context.
ConcurrencyDiagnosticBehavior
getConcurrencyDiagnosticBehaviorLimit(NominalTypeDecl *nominal,
                                      const DeclContext *fromDC,
                                      bool ignoreExplicitConformance = false);

/// Determine whether the given nominal type has an explicit Sendable
/// conformance (regardless of its availability).
bool hasExplicitSendableConformance(NominalTypeDecl *nominal,
                                    bool applyModuleDefault = true);

} // namespace swift

#endif
