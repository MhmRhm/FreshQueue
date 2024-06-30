# The FreshQueue

Demonstrating a Benchmarking Pipeline

CI/CD pipelines with automated testing have been well-established. In this
article, I will make the case for including benchmarks and argue why they
deserve the same level of attention as automated tests, if not more.

When starting a project, we set guidelines for functionality and performance.
Automated tests ensure our code behaves correctly and hopefully handles all
possible situations. They simplify software testing to the point of pressing a
single button. Additionally, they are invaluable in Test-Driven Development
(TDD), where tests are written first to set expectations for code interaction
and scenario coverage.

The benefits of automated testing are well known, but the same argument is not
so obvious for benchmarks. Imagine a customer with a heavy workload starts
complaining that your product has become slow. Without preexisting benchmarks,
you would have to go through a long list of commits without knowing where to
start. Testing the software in this scenario would be challenging, and even
then, you might not know which baseline to compare your benchmark results
against.

The original team members who designed the performance-critical parts of the
software may no longer be around, having been replaced by new team members, who
in turn may be replaced again. Each group can only compare the software's
performance to what they experienced during their early days with the project.

As a result, you may find yourself in a situation where you don't know which
part of the software is causing the slowdown, which commits have introduced the
issue, how to measure its performance, or, even if you manage to do all that,
which baseline to use for comparison.

On the other hand, if you had automated benchmarks in place from day one, even
years later, new team members would have a clear understanding of the software's
performance history and how it has improved or deteriorated over time. They
would be immediately alerted if a commit caused noticeable performance drops.
Most importantly, they would have a baseline for comparison and a target for
maintaining or improving performance.

To that end, I will explain the steps you need to follow to incorporate a
benchmarking workflow into your CI/CD pipelines.
