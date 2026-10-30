#!/usr/bin/env python3
"""Shared rules for the strict RISC-V historical-best audit."""

from __future__ import annotations

import statistics
from dataclasses import dataclass
from typing import Sequence


@dataclass(frozen=True)
class StrictVerdict:
    """Final classification for one set of valid paired measurements."""

    baseline_wins: int
    reference_wins: int
    ties: int
    baseline_median_us: float
    reference_median_us: float
    baseline_average_us: float
    reference_average_us: float
    classification: str
    needs_second_batch: bool


@dataclass(frozen=True)
class BatchPrune:
    """Result of applying the paired anomaly rule to a candidate batch."""

    accepted_indices: tuple[int, ...]
    discarded_indices: tuple[int, ...]
    refill_pairs: int
    batch_unstable: bool


def relative_deviation(value: float, median: float) -> float:
    """Return the absolute relative deviation, handling a zero median."""

    if median == 0:
        return 0.0 if value == 0 else float("inf")
    return abs(value - median) / abs(median)


def outlier_pair_indices(
    pairs: Sequence[tuple[float, float]], threshold: float
) -> set[int]:
    """Find pairs where either side is too far from its own batch median."""

    if not pairs:
        return set()
    baseline_median = statistics.median(pair[0] for pair in pairs)
    reference_median = statistics.median(pair[1] for pair in pairs)
    return {
        index
        for index, (baseline, reference) in enumerate(pairs)
        if relative_deviation(baseline, baseline_median) > threshold
        or relative_deviation(reference, reference_median) > threshold
    }


def prune_candidate_batch(
    pairs: Sequence[tuple[float, float]],
    required_pairs: int,
    threshold: float,
    discarded_so_far: int,
    max_discarded: int,
) -> BatchPrune:
    """Apply whole-pair rejection and report how many pairs must be refilled."""

    if len(pairs) < required_pairs:
        return BatchPrune(
            accepted_indices=tuple(range(len(pairs))),
            discarded_indices=(),
            refill_pairs=required_pairs - len(pairs),
            batch_unstable=False,
        )
    discarded = tuple(sorted(outlier_pair_indices(pairs, threshold)))
    accepted = tuple(
        index for index in range(len(pairs)) if index not in discarded
    )
    return BatchPrune(
        accepted_indices=accepted,
        discarded_indices=discarded,
        refill_pairs=required_pairs - len(accepted),
        batch_unstable=discarded_so_far + len(discarded) > max_discarded,
    )


def classify_pairs(
    pairs: Sequence[tuple[float, float]], required_pairs: int = 13
) -> StrictVerdict:
    """Classify an already validated batch using the strict audit rules."""

    if len(pairs) != required_pairs:
        raise ValueError(
            f"expected {required_pairs} valid pairs, got {len(pairs)}"
        )
    baseline = [pair[0] for pair in pairs]
    reference = [pair[1] for pair in pairs]
    baseline_wins = sum(lhs < rhs for lhs, rhs in pairs)
    reference_wins = sum(lhs > rhs for lhs, rhs in pairs)
    ties = required_pairs - baseline_wins - reference_wins
    baseline_median = float(statistics.median(baseline))
    reference_median = float(statistics.median(reference))
    baseline_average = float(statistics.fmean(baseline))
    reference_average = float(statistics.fmean(reference))
    aggregates_pass = (
        baseline_median < reference_median
        and baseline_average < reference_average
    )

    if not aggregates_pass or baseline_wins <= 8:
        classification = "not_best"
    elif baseline_wins == 13:
        classification = "stable_dominance"
    elif baseline_wins >= 11:
        classification = "strong_dominance"
    else:
        classification = "strict_best"

    return StrictVerdict(
        baseline_wins=baseline_wins,
        reference_wins=reference_wins,
        ties=ties,
        baseline_median_us=baseline_median,
        reference_median_us=reference_median,
        baseline_average_us=baseline_average,
        reference_average_us=reference_average,
        classification=classification,
        needs_second_batch=classification == "strict_best",
    )
