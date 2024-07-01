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

To that end, in a case study, I will explain the steps you need to follow to
incorporate a benchmarking workflow into your CI/CD pipelines.

There are many parts involved in this case study:
- The code being benchmarked
- The tests and benchmarking code
- The Git server that runs the CI/CD pipeline
- The CI/CD pipeline and workflows
- The actors that build, test, and benchmark the codebase
- The Docker images used for or by the actors
- The tools used for analyzing the benchmark results

All these pieces come together to form our pipeline. We will go through them one
by one and examine the code and configuration files needed for this case study.

## The Code

We will test and benchmark a few implementations of a concurrent queue. For more
detailed information on concurrency and the implementation of these data
structures, refer to
[C++ Concurrency in Action](https://www.manning.com/books/c-plus-plus-concurrency-in-action).
The full version of the code discussed here is available at
[FreshQueue](https://github.com/MhmRhm/FreshQueue).

Our first implementation is a concurrent queue that uses locks to manage
simultaneous pushes and pops from multiple threads.

```c++
template <typename T> class ThreadSafeFreshQueue {
public:

    ...

  void push(T val) {
    const std::lock_guard lock{m_mutex};
    m_queue.push(std::make_shared<T>(std::move(val)));
    m_pushNotification.notify_one();
  }

  std::shared_ptr<T> tryPop() {
    const std::lock_guard lock{m_mutex};
    if (m_queue.empty())
      return {};
    auto result{m_queue.front()};
    m_queue.pop();
    return result;
  }

  std::shared_ptr<T> waitAndPop() {
    std::unique_lock uniqueLock{m_mutex};
    m_pushNotification.wait(uniqueLock, [&] { return !m_queue.empty(); });
    auto result{m_queue.front()};
    m_queue.pop();
    return result;
  }

  ...

};
```

The second implementation is an enhanced singly linked list with separate locks
on the head and tail to reduce contention on shared data. We expect this data
structure to be more performant than our first one when accessed by multiple
threads.

```c++
template <typename T> class ConcurrentFreshQueue {
private:
  struct Node {
    std::shared_ptr<T> data;
    std::unique_ptr<Node> next;
  };

  ...

  bool tryPopHead(T &value) {
    const std::lock_guard headLock{m_headMutex};
    if (m_head.get() == getTail()) {
      return false;
    }
    value = std::move(*popHead()->data);
    return true;
  }

  ...

public:
  void push(const T &value) {
    auto newTail{std::make_unique<Node>()};
    auto newTailRaw = newTail.get();
    auto newData = std::make_shared<T>(std::move(value));
    {
      std::lock_guard tailLock{m_tailMutex};
      m_tail->data = newData;
      m_tail->next = std::move(newTail);
      m_tail = newTailRaw;
    }
    m_pushNotification.notify_one();
  }

  std::shared_ptr<T> tryPop() {
    auto head{tryPopHead()};
    if (head) {
      return head->data;
    }
    return {};
  }

  ...

};
```

Lastly, we utilize an open-source lock-free queue implementation from the
[Boost library](https://www.boost.org/doc/libs/1_85_0/boost/lockfree/queue.hpp).

## Tests and Benchmarks

For these tasks, we utilize Google libraries available at
[GoogleTest](http://google.github.io/googletest/) and
[Google Benchmark](https://github.com/google/benchmark/blob/main/docs/user_guide.md).
Here's an example of the benchmarking code. It may seem overwhelming at first,
but I'll break it down for you.

```c++
template <typename T>
class BM_LockFreeFreshQueueMultiThreadFixture : public benchmark::Fixture {
protected:
  boost::lockfree::queue<int> m_queue{10};
};
BENCHMARK_TEMPLATE_DEFINE_F(BM_LockFreeFreshQueueMultiThreadFixture, PushAndPop,
                            int)
(benchmark::State &state) {
  bool isPushingThread{state.thread_index() % 2 == 0};
  if (isPushingThread) {
    for (auto _ : state) {
      m_queue.push(42);
    }
    state.counters["Pushes"] = benchmark::Counter(
        static_cast<int64_t>(state.iterations()), benchmark::Counter::kIsRate);
  } else {
    int value{};
    for (auto _ : state) {
      while (!m_queue.pop(value))
        ;
      benchmark::DoNotOptimize(value);
    }
  }
}
BENCHMARK_REGISTER_F(BM_LockFreeFreshQueueMultiThreadFixture, PushAndPop)
    ->ThreadRange(2, 1 << 10)
    ->MeasureProcessCPUTime()
    ->UseRealTime();
```

Here's the main part of the code that we're benchmarking:

```c++
if (isPushingThread) {
  m_queue.push(42);
} else {
  int value{};
  while (!m_queue.pop(value));
}
```

In a pushing thread, we push data to the queue; otherwise, we pop data from it.
Here's a Google Benchmark code snippet that executes the tested code repeatedly
to measure its performance across multiple runs:

```c++
for (auto _ : state) {
  ...
}
```

The `state` variable keeps track of the number of times a piece of code is
executed and the time taken for each execution. We add to its measurement
capabilities using the following:

```c++
state.counters["Pushes"] = benchmark::Counter(
  static_cast<int64_t>(state.iterations()), benchmark::Counter::kIsRate);
```

Finally, we instruct the Google Benchmark library on the number of threads to
use when executing our code:

```c++
BENCHMARK_REGISTER_F(BM_LockFreeFreshQueueMultiThreadFixture, PushAndPop)
  ->ThreadRange(2, 1 << 10)
```

The output from this will look like the following:

```c++
.../threads:2            77.4 ns          155 ns     10036644 Pushes=6.45765M/s
.../threads:4             148 ns          593 ns      4214136 Pushes=3.36932M/s
.../threads:8             254 ns         2029 ns      2775024 Pushes=1.96716M/s
.../threads:16            346 ns         3436 ns      1600000 Pushes=1.44463M/s
.../threads:32            349 ns         3475 ns      2341856 Pushes=1.43367M/s
.../threads:64            358 ns         3575 ns      2196800 Pushes=1.39629M/s
.../threads:128           341 ns         3405 ns      2263040 Pushes=1.46528M/s
.../threads:256           258 ns         2579 ns      2560000 Pushes=1.93559M/s
.../threads:512           274 ns         2743 ns      5120000 Pushes=1.82198M/s
.../threads:1024          312 ns         3124 ns     10240000 Pushes=1.60068M/s
```

Now that we have our code and its benchmarking in place, it's time to move on to
the next step, which is setting up our Git server and CI/CD pipelines.

## Git Server

We use Gitea as our Git server because it is open-source, free, lightweight
compared to other solutions like GitLab, and easy to configure. To set up a
local Gitea instance, you can use the following docker-compose.yml file:

```yml
services:
  gitea:
    image: 'gitea/gitea:latest'
    container_name: gitea
    environment:
      - USER_UID=1000
      - USER_GID=1000
      - GITEA__database__DB_TYPE=postgres
      - GITEA__database__HOST=postgres_gitea:5432
      - GITEA__database__NAME=gitea
      - GITEA__database__USER=gitea
      - GITEA__database__PASSWD=gitea
    restart: always
    ports:
      - '3000:3000'
      - '2222:22'
    volumes:
      - ./gitea:/data
      - /etc/timezone:/etc/timezone:ro
      - /etc/localtime:/etc/localtime:ro
    depends_on:
      postgres_gitea:
        condition: service_healthy

  postgres_gitea:
    image: 'postgres:latest'
    container_name: postgres_gitea
    restart: always
    environment:
      - POSTGRES_USER=gitea
      - POSTGRES_PASSWORD=gitea
      - POSTGRES_DB=gitea
    volumes:
      - ./gitea/postgres:/var/lib/postgresql/data
    healthcheck:
      test: ["CMD", "pg_isready"]
      interval: 3s
      timeout: 5s
      retries: 3

  runner_gitea:
    image: gitea/act_runner:nightly
    container_name: runner_gitea
    environment:
      GITEA_INSTANCE_URL: "http://<server-ip>:3000"
      GITEA_RUNNER_REGISTRATION_TOKEN: "<from-gitea>"
      GITEA_RUNNER_NAME: "Docker-Runner"
      GITEA_RUNNER_LABELS: "docker-runner"
    volumes:
      - ./gitea/runner:/data
      - /var/run/docker.sock:/var/run/docker.sock
```

This setup will configure three containers: one for the Gitea instance, another
for the Gitea database, and a third for the Gitea runner. For the runner to
function correctly, you need to first launch the Gitea instance, extract a
registration token from it, and then provide that token to the runner.

The first time you run Gitea, it will prompt you for initial setup settings.
While you can skip most of them, ensure to create an administrative account.
This account may be necessary for enabling Actions for the instance later on.

Once Gitea setup is complete, create a user account for yourself and a
repository to push your codebase to. After setting up the repository, navigate
to its settings and under *Actions->Runners*. Use *Create new Runner* to obtain
the Registration Token. Paste this token into the docker-compose.yml file, then
start the runner with `sudo docker compose up -d`. You should now see the runner
listed under *Actions -> Runners*.

<p align="center"><img src="https://github.com/MhmRhm/FreshQueue/blob/main/doc/images/actions_runners.png" alt="Actions-Runners"></img></p>

## CI/CD Pipeline

To create a workflow, add a .yml file to the .gitea/workflows directory in your
repository. The syntax of this file is the same as
[GitHub Actions](https://docs.github.com/en/actions). Below is an example action
used to run both tests and benchmarks in our case study. 

One part is particularly important: the `runs-on:` setting. Here, you list the
labels used when setting up the runners, such as
`GITEA_RUNNER_LABELS: "docker-runner"` in our docker-compose.yml file. Any
runner with any of the specified labels will be eligible to run the workflow.

```yml
name: Run Tests and Benchmarks
run-name: ${{ gitea.actor }} Running Tests and Benchmarks Actions
on: [push]

jobs:
  Tests:
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

      - name: Run Tests
        run: ./test/infrastructure/infrastructure_test
        working-directory: ${{ gitea.workspace }}-build-linux-default-release

  Benchmarks:
    needs: Tests
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

## Actors

We have already set up an actor in our docker-compose.yml file. However, this
approach can have some issues. The images provided by Gitea or GitHub for use as
actors may not always be up-to-date. For example, in our case, we need a recent
version of CMake to configure and build our project. Unfortunately, these
distributions are often a few versions behind.

Another issue is that if we use the provided images, we have to install the
necessary dependencies for our project with each run of the workflow, which can
be time-consuming. More importantly, our benchmarks need to be run on a stable
system. Since both our Gitea instance and runner are running on the same machine
, the server might be used for other tasks while the runner is benchmarking a
build. This can lead to unstable and inaccurate benchmark results. Additionally,
if someone is developing code for a specific line of GPUs, they will need access
to the actual hardware.

The solution to all these problems is to set up a dedicated physical machine
solely for running benchmarks. You can also lock the CPU frequency on that
machine to achieve even more stable results. This way, you only need to
configure the system once.

To set up a machine as a runner, you need to download the Act Runner and
configure it properly. This process is quite straightforward. The following
steps outline how to set up a dedicated machine for this case study.

For more detailed explanations, you can refer to the following links:

- [Act Runner](https://docs.gitea.com/usage/actions/act-runner)
- [CMake](https://apt.kitware.com)
- [Lock Freq](https://forums.linuxmint.com/viewtopic.php?p=1880419&sid=ac3c263e5d659e366c34f66c48ef8888#p1880419)
- [NVM](https://github.com/nvm-sh/nvm)

```bash
# install requirements
sudo apt-get update && sudo apt-get -y upgrade
sudo apt-get -y install build-essential clang clang-format clang-tidy \
  clang-tools cmake doxygen graphviz cppcheck valgrind lcov libssl-dev \
  ninja-build libtbb-dev libboost-all-dev

# install cmake
sudo apt-get install ca-certificates gpg wget
test -f /usr/share/doc/kitware-archive-keyring/copyright ||
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ jammy main' | sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null
sudo apt-get update
test -f /usr/share/doc/kitware-archive-keyring/copyright ||
sudo rm /usr/share/keyrings/kitware-archive-keyring.gpg
sudo apt-get install kitware-archive-keyring
sudo apt-get autoremove cmake
sudo apt-get install cmake

# lock freq
sudo apt-get install cpufrequtils
sudo cpufreq-set --governor userspace
sudo cpufreq-set --freq 1800000

# install runner
sudo su -
wget -qO- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.7/install.sh | bash
exit
sudo su -
nvm install node
wget https://dl.gitea.com/act_runner/nightly/act_runner-nightly-linux-arm64
chmod +x act_runner-nightly-linux-arm64
./act_runner-nightly-linux-arm64 --version
./act_runner-nightly-linux-arm64 register --no-interactive --instance \
  'http://<ip-address>:3000' \
  --token '<from-gitea>' --name Custom-Machine \
  --labels 'arm-locked-freq-linux'
./act_runner-nightly-linux-arm64 daemon
```

## Comparison

The final step is to compare the benchmarking results to a baseline. I used a
Python script to compare the results for each benchmark. Additionally, the
Google Benchmark library provides a more comprehensive
[tool](https://github.com/google/benchmark/blob/main/docs/tools.md) for
comparing results and performing statistical analysis.

```python
import json
import sys

if len(sys.argv) != 2:
    print('Usage: python3 compare.py <workflow_run>.json')
    sys.exit(1)

with open(sys.argv[1], 'r') as f:
    workflow_json = json.load(f)

workflow_map = {}
for benchmark in workflow_json['benchmarks']:
    workflow_map[benchmark['name']] = benchmark['Pushes']

with open('baseline.json', 'r') as f:
    baseline_json = json.load(f)

baseline_map = {}
for benchmark in baseline_json['benchmarks']:
    baseline_map[benchmark['name']] = benchmark['Pushes']

deteriorated_benchmarks = list()
for name, pushes in baseline_map.items():
    if name in workflow_map:
        if pushes > workflow_map[name] and (pushes - workflow_map[name]) / pushes > 0.05:
            deteriorated_benchmarks.append((name, (pushes - workflow_map[name]) / pushes))
            result = False

if deteriorated_benchmarks:
    print(*deteriorated_benchmarks, sep='\n')
    exit(-1)
exit(0)
```

We placed this script in our workflows directory. Additionally, we need to
provide it with a baseline file, which in this case is the result from a
benchmark run used as the baseline.

Now, the pipeline is ready and will run on each push:

<p align="center"><img src="https://github.com/MhmRhm/FreshQueue/blob/main/doc/images/pipeline_run.png" alt="Pipeline Run"></img></p>
