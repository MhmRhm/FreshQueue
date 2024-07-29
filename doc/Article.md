# Benefits of a Benchmarking Pipeline

## Preface

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
so obvious for benchmarks. Imagine a customer with a heavy workload complaining
that your product has become slow. Without preexisting benchmarks, you would
have to go through a long list of commits without knowing where to start.
Testing the software in this scenario would be challenging, and even then, you
might not know which baseline to compare your benchmark results against.

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

## Required Components

To that end, I will explain the steps you need to follow to
incorporate a benchmarking workflow into your CI/CD pipelines.

There are many parts involved in a benchmarking pipeline:

- The code being benchmarked
- The tests and benchmarking code
- The Git server that runs the CI/CD pipeline
- The CI/CD pipeline and workflows
- The actors that build, test, and benchmark the codebase
- The Docker images used for or by the actors
- The tools used for analyzing the benchmark results

All these components combine to form our pipeline. Let's briefly review them,
explain the available options, and highlight which ones help you get started
faster.

### The Code and Benchmark Library

With benchmarking in mind, the code should be written in a way that allows for
effective benchmarking. If you are practicing Test Driven Development (TDD) or
have a testable codebase, it usually means it is also suitable for benchmarking.
Concurrent code is typically designed for performance, but it is also the most
difficult to debug. Therefore, it is crucial to consider benchmarking from the
start. Modifying and refactoring these parts of your software can be very risky.

For the ones who work with C/C++, the
[Google Benchmark](https://github.com/google/benchmark) library offers many
features, including the ability to:

- Define a range of threads to run the code on
- Specify a range of inputs for the code
- Combine both thread and input settings
- Create templated benchmarking code
- Perform statistical analysis on benchmarking results

### Git Server and CI/CD Pipeline

For our Git server, we have several options available, with
[GitHub](https://github.com/) being a prominent one. You may also want to
consider [Gitea](https://gitea.com/) for several reasons:

1. **Self-Hosting**: Unlike GitHub, Gitea allows us to host the server on our
own network.
2. **Open Source and Free**: Gitea is open source and free to use.
3. **Features**: It comes with many features, including automated pipelines,
issue tracking, and code review.
4. **Lightweight**: Unlike a self-hosted [GitLab](https://about.gitlab.com/)
instance, Gitea is much more lightweight and has a smaller performance footprint
on the server, allowing more resources for other functionalities.
5. **Quick Setup**: The setup can be done within seconds using Docker images.

Another benefit of using Gitea is that the syntax for CI/CD workflows is the
same as GitHub's, which is widely used and very well documented.

Here is an example of a typical benchmarking pipeline:

```yml
name: Run Benchmarks
run-name: ${{ gitea.actor }} Running Benchmarks Actions
on: [push]

jobs:
  Benchmarks:
    runs-on: arm-locked-freq-linux
    steps:
      - name: Checkout Repository
        uses: actions/checkout@v4

      - name: Configure
        run: cmake --preset linux-default-release
        working-directory: ${{ gitea.workspace }}

      - name: Build
        run: cmake --build --preset linux-default-release
        working-directory: ${{ gitea.workspace }}

      - name: Cache Warm-Up
        run: ./benchmark/infrastructure/infrastructure_benchmark
        working-directory: ${{ gitea.workspace }}-build-linux-default-release

      - name: Run Benchmarks
        run: ./benchmark/infrastructure/infrastructure_benchmark --benchmark_out=${{ gitea.sha }}_${{ gitea.run_number }}.json --benchmark_out_format=json
        working-directory: ${{ gitea.workspace }}-build-linux-default-release

      - name: Compare Benchmarks
        run: python3 compare.py ${{ gitea.workspace }}-build-linux-default-release/${{ gitea.sha }}_${{ gitea.run_number }}.json
        working-directory: ${{ gitea.workspace }}/.gitea/workflows/
```

### Actors

Actors are the virtual machines (VMs) or physical machines that build and run
your code to test or benchmark.

We can easily set up an actor using a Docker image. However, this approach can
have some issues. The Docker images provided by Gitea or GitHub for use as actors
may not always be up-to-date. For example, you might need a recent version of
CMake to configure and build a project with C++20's modules support, but these
Docker images are often a few versions behind.

More importantly, our benchmarks need to be run on a stable
system. Since both our Gitea instance and runner are running on the same machine
, the server might be used for other tasks while the runner is benchmarking a
build. This can lead to unstable and inaccurate benchmark results. Additionally,
if someone is developing code for a specific line of GPUs, they will need access
to the actual hardware.

The solution to all these problems is to set up a dedicated physical machine
solely for running benchmarks. You can also lock the CPU frequency on that
machine to achieve even more stable results.

To set up a machine as a runner, you need to download the Act Runner and
configure it properly. This process is quite straightforward.

### Comparison

The final step is to compare the benchmarking results to a baseline. Since you
can have the benchmarking results in CSV or JSON format, you may use a Python
script to compare the results for each benchmark. Additionally, the Google
Benchmark library provides a more comprehensive tool for comparing results and
performing statistical analysis.

Now, the pipeline is ready and will run on each push:

<p align="center"><img src="https://github.com/MhmRhm/FreshQueue/blob/main/doc/images/pipeline_run.png" alt="Pipeline Run"></img></p>

## Closing Thoughts

We explored why tracking software performance is essential and what components
are necessary for a pipeline to achieve this. When dealing with performance, it's
always a good practice to aim higher than the requirements. For example, if a
process needs to run in 100 ms, aim for 70 or 80 ms. You will be surprised at the
innovative ways you can achieve this. This approach leaves you with a buffer of
20 to 30 ms for future requirements or unexpected challenges.

With a functioning and well-maintained CI/CD pipeline in place, you can ensure
that the criteria on which the code was developed will remain in place long
after you are gone. This provides assurance to marketing and management that
such tools will offer valuable insights into software performance and trends.
