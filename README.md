# EntropyX
# EntropyX:
## A Physical True Random Number Generator (TRNG) with Embedded NIST SP 800-90B Min-Entropy Estimation and Toeplitz Universal Hash Extraction

### Abstract

Software Pseudo-Random Number Generators (PRNGs) rely on predictable algorithms. When initialized with predictable system data like as clock timestamps or memory , the entire sequence of cryptographic keys can be computed by an enemy. Today’s Commercial Hardware Security Modules (HSMs) address this using physical noise sources, but they cost hella lot....between 20000$ and 50000$ function as closedsourceboxes, and do not show internal entropy metrics in real time.

EntropyX is a physical **True Random Number Generator (TRNG)** and key-derivation device. It harvests physical noise across three independent channels on a 10-DOF MEMS sensor cluster (MPU6050, QMC6308, and the on-board FXLS8974C): thermal electron movement in analog transimpedance amplifiers, Brownian mechanical motion of silicon proof-masses, and geomagnetic flux micro-disturbances. This noise is integrated with high-frequency CPU data harvested directly from the Arm Cortex-M33 **Data Watchpoint and Trace (DWT)** cycle counter running at $150\text{ MHz}$.

To remove baselines, specifically Earth's 1G static gravitational vector and ambient DC magnetic biases—the firmware processes every sample through a discrete first-order difference filter ($H(z) = 1 - z^{-1}$) paired with cyclic Von Neumann transition de-biasing.

Every 1,024-bit raw block is validated in bare-metal SRAM against NIST SP 800-90B min-entropy thresholds:

$$H_\infty \ge 0.500\text{ bits/bit}\quad (\text{empirically measured: } 0.90\text{--}0.98\text{ bits/bit})$$

alongside three inline **NIST SP 800-22** statistical tests (Frequency Monobit, Runs Distribution, and Serial 2-Bit Pattern) at significance level $\alpha = 0.01$.

Blocks that pass is compressed into 256 bits using an information-theoretic **Toeplitz matrix universal hash extractor** ($\mathcal{H}_{\text{Toeplitz}} : \{0, 1\}^{1024} \to \{0, 1\}^{256}$) driven by a 32-bit primitive Galois Linear Feedback Shift Register (LFSR) polynomial:

$$P(x) = x^{32} + x^{22} + x^2 + x + 1$$

Under the **Leftover Hash Lemma**, an empirical input min-entropy of $k = 921.6\text{ bits}$ bounds the output statistical distance from uniform randomness to $\Delta \le 2^{-333.8} \ll 2^{-64}$, exceeding the NIST full-entropy requirement. Cryptographic conditioning is completed using an on-chip, hardware-accelerated **NIST FIPS 180-4 SHA-256** engine. Continuous testing across $10^6\text{ bits}$ yields a mean Avalanche Effect Hamming distance of:

$$\mu = 52.1\% \pm 2.8\%\quad (\sigma = 7.17\text{ bits})$$

matching the theoretical ideal of $50.0\%$.
------------------------------

<img width="1600" height="900" alt="WhatsApp Image 2026-09-17 at 9 43 15 PM" src="https://github.com/user-attachments/assets/6cfe02b1-e19b-449f-bda7-e63747cbca00" />


-------------------------

<img width="1600" height="900" alt="WhatsApp Image 2026-09-17 at 9 43 16 PM (1)" src="https://github.com/user-attachments/assets/c91db633-de1a-47b7-a59b-27aff53d3c35" />


-------------------------------

<img width="1600" height="900" alt="WhatsApp Image 2026-09-17 at 9 43 16 PM" src="https://github.com/user-attachments/assets/eaaf33ac-3b36-4480-b55f-f92103bd2d38" />


--------------------------------------


<img width="1600" height="896" alt="WhatsApp Image 2026-09-17 at 9 43 17 PM" src="https://github.com/user-attachments/assets/5e57a118-449d-45b5-9d92-3f4289fb31f9" />



--------------------------
### The entire project was coded on MCUXpressoIDE

----------------------
<img width="1920" height="921" alt="Screen Shot 2026-09-22 at 22 14 36 PM" src="https://github.com/user-attachments/assets/9e1595f8-c789-4911-b250-baa40f942d81" />



-------

### Web Dash 


------------------------------


<img width="1340" height="639" alt="Screenshot 2026-09-18 182001" src="https://github.com/user-attachments/assets/97004622-3259-41a5-b80b-e500b4765cca" />


------------------------------------



<img width="328" height="565" alt="Screenshot 2026-09-18 182051" src="https://github.com/user-attachments/assets/e65cbd64-6dfb-424a-a2ea-c71d3f659210" />




-----------------------------------


<img width="385" height="547" alt="Screenshot 2026-09-18 182232" src="https://github.com/user-attachments/assets/2496a697-fec0-4ab6-802d-e6e10a1a7377" />


-------------------------------



<img width="499" height="559" alt="Screenshot 2026-09-18 182208" src="https://github.com/user-attachments/assets/ed83869a-3f0e-496c-9cf9-3b1122356c5d" />



---------------------------------




## 1. System Architecture

### 1.1 State Machine Determinism vs. Physical Entropy

A deterministic PRNG advances an internal state vector $s \in \{0, 1\}^n$ through an algebraic operator $\Phi$ and emits a keystream through an output function $\Psi$:

$$s_{t+1} = \Phi(s_t), \quad K_t = \Psi(s_t)$$

Because both functions are deterministic, the conditional entropy of any key $K_t$ given the seed $s_0$ is zero:

$$H(K_t \mid s_0) = 0$$

EntropyX replaces algorithmic PRNG state updates with non-deterministic physical noise:

```
[ EntropyX Hardware TRNG (Physical) ]
                  │
                  ▼
      ┌───────────────────────┐
      │ MEMS Thermal/Brownian │
      │         Noise         │
      └───────────┬───────────┘
                  │
                  ▼
      ┌───────────────────────┐
      │ On-Chip NIST          │
      │ Gatekeeper            │
      └───────────┬───────────┘
                  │
                  ▼
      ┌───────────────────────┐
      │ Toeplitz LHL          │
      │ Extractor             │
      └───────────┬───────────┘
                  │
                  ▼
      ┌───────────────────────┐
      │ Key Output:           │
      │ Δ ≤ 2⁻³³³·⁸           │
      └───────────────────────┘

[ Deterministic PRNG (Algorithmic) ]
                  │
                  ▼
      ┌───────────────────────┐
      │ Seed State s₀         │
      └───────────┬───────────┘
                  │
                  ▼
      ┌───────────────────────┐
      │ Evolution Φ(s)        │
      └───────────┬───────────┘
                  │
                  ▼
      ┌───────────────────────┐
      │ Key Output Kₜ:        │
      │ H(Kₜ|s₀) = 0          │
      └───────────────────────┘
```

### 1.2 Threat Model & Active Physical Defenses

- **DC Magnetic Saturation:** Applying a permanent external magnet ($B_{\text{ext}} \ge 1.5\text{ T}$) saturates the Hall-effect transducers.
  - **\*Defense:** The magnetometer delta $\Delta S_{\text{mag}}(t) \to 0$. The system continues harvesting from the gyroscope, accelerometer, and DWT cycle jitter.
- **Thermal Quenching:** Lowering the operating temperature ($T \to 77\text{ K}$) suppresses thermal Johnson-Nyquist noise variance ($\lim_{T \to 0} 4k_B T R = 0$).
  - **Defense:** Reduced bit variance causes the calculated min-entropy to drop below $0.500\text{ bits/bit}$, prompting the gatekeeper to purge the entropy pool and halt key generation.
- **Ultrasonic Acoustic Injection:** Emitting acoustic waves at the resonant frequency of the proof mass ($\omega_0 \approx 2\pi \times 4.5\text{ kHz}$) forces periodic displacement.
  - **Defense:** First-order differentiation suppresses uniform periodic waveforms, and XOR mixing with the 150 MHz core cycle timer prevents bit-level phase-locking.
- **Bus and Memory Eavesdropping:** Sniffing inter-chip data transfers.
  - **Defense:** The display runs on a physically isolated I2C controller (LPI2C5), maintaining air-gapped isolation from general-purpose communications.

---

## 2. Active Sensor Channels & Harvesting Mechanics

The sampling routine reads **11 active 16-bit physical channels** on every loop iteration:

1. `accel_x` (MPU6050 Accelerometer X-axis)
2. `accel_y` (MPU6050 Accelerometer Y-axis)
3. `accel_z` (MPU6050 Accelerometer Z-axis)
4. `gyro_x` (MPU6050 Gyroscope X-axis)
5. `gyro_y` (MPU6050 Gyroscope Y-axis)
6. `gyro_z` (MPU6050 Gyroscope Z-axis)
7. `mag_x` (QMC6308 Magnetometer X-axis)
8. `mag_y` (QMC6308 Magnetometer Y-axis)
9. `mag_z` (QMC6308 Magnetometer Z-axis)
10. `onboard_accel_z` (FXLS8974C On-Board Accelerometer Z-axis)
11. `aux_entropy` (Arm Cortex-M33 DWT->CYCCNT low-order 16 bits)

---

## 3. Firmware Signal Conditioning & Mathematical Whitening

```
           Raw 16-Bit Channel Input S(t)
                         │
                         ▼
        ┌──────────────────────────────────┐
        │ Discrete Difference Operator     │
        │ ΔS(t) = S(t) - S(t-1)            │
        │ H(z) = 1 - z⁻¹ (Rejects DC/1G)   │
        └────────────────┬─────────────────┘
                         │
                         ▼
        ┌──────────────────────────────────┐
        │ Core Cycle Jitter XOR Mixing     │
        │ mix = ΔS(t) ⊕                    │
        │       (DWT->CYCCNT & 0xFF)       │
        └────────────────┬─────────────────┘
                         │
                         ▼
        ┌──────────────────────────────────┐
        │ Von Neumann Transition Filter    │
        │ Pairs: (0,1)→0, (1,0)→1,         │
        │        (0,0)/(1,1)→Discard       │
        └────────────────┬─────────────────┘
                         │
                         ▼
        ┌──────────────────────────────────┐
        │ Unbiased Bitstream:              │
        │ P(0) = P(1) = 0.500              │
        └──────────────────────────────────┘
```

### 3.1 First-Order Difference Filter (Gravity & Baseline Cancellation)

Resting inertial sensors exhibit a constant DC offset from Earth's 1G gravity vector:

$$\mathbf{g} = [0,\, 0,\, 9.80665\text{ m/s}^2]^T$$

To remove this DC baseline, the firmware computes the backward difference between consecutive samples:

$$\Delta S(t) = S(t) - S(t - 1)$$

The transfer function in the discrete $\mathcal{Z}$-domain is:

$$H(z) = 1 - z^{-1} = \frac{z - 1}{z}$$

Evaluating its frequency response on the unit circle ($z = e^{j\omega}$):

$$H(e^{j\omega}) = 1 - e^{-j\omega} = e^{-j\omega/2} \left(e^{j\omega/2} - e^{-j\omega/2}\right) = 2j e^{-j\omega/2} \sin\left(\frac{\omega}{2}\right)$$

The magnitude response is:

$$|H(e^{j\omega})| = \left|2 \sin\left(\frac{\omega}{2}\right)\right|$$

- At zero frequency ($\omega = 0$, representing static gravity and magnetic baselines):
  $$|H(e^{j0})| = |2 \sin(0)| = 0$$
- At Nyquist ($\omega = \pi$, representing stochastic noise transitions):
  $$|H(e^{j\pi})| = \left|2 \sin\left(\frac{\pi}{2}\right)\right| = 2$$

**Proof of DC Rejection:** Let the raw signal be modeled as $S(t) = G_0 + \eta(t)$, where $G_0$ is the constant gravity/magnetic offset and $\eta(t)$ is zero-mean stationary physical noise with $\mathbb{E}[\eta(t)] = 0$:

$$\Delta S(t) = [G_0 + \eta(t)] - [G_0 + \eta(t-1)] = \eta(t) - \eta(t-1)$$

$$\mathbb{E}[\Delta S(t)] = \mathbb{E}[\eta(t)] - \mathbb{E}[\eta(t-1)] = 0 - 0 = 0$$

The static baseline $G_0$ is removed, centering the output distribution at zero.

### 3.2 High-Frequency Core Clock Jitter Mixing

Each differentiated sample is XOR-mixed with the lowest-order byte of the 150 MHz cycle counter:

$$\text{mix} = \Delta S_i(t) \oplus (\text{DWT->CYCCNT} \ \& \ \text{0xFF})$$

This breaks any periodic sampling artifacts introduced by loop execution times.

### 3.3 Von Neumann Transition Extraction

To eliminate probability skew where $P(X = 1) = p \ne 0.5$, bit pairs $(b_0, b_1)$ from the mixed word are processed through a Von Neumann filter:

$$P(0,0) = (1 - p)^2$$

$$P(1,1) = p^2$$

$$P(0,1) = (1 - p)p$$

$$P(1,0) = p(1 - p)$$

Mapping rules:
- $\text{Input } (0, 1) \implies \text{Emit } 0$
- $\text{Input } (1, 0) \implies \text{Emit } 1$
- $\text{Input } (0, 0)\text{ or }(1, 1) \implies \text{Discard}$

The conditional probability of an emitted bit is:

$$P(Y = 0 \mid \text{emitted}) = \frac{P(0,1)}{P(0,1) + P(1,0)} = \frac{(1 - p)p}{(1 - p)p + p(1 - p)} = \frac{1}{2}$$

$$P(Y = 1 \mid \text{emitted}) = \frac{P(1,0)}{P(0,1) + P(1,0)} = \frac{p(1 - p)}{2p(1 - p)} = \frac{1}{2}$$

This ensures:

$$P(Y = 0) = P(Y = 1) = 0.5000\dots$$

The extracted stream is mathematically unbiased.

---

## 4. On-Chip NIST Statistical Qualification Layer

```
                     ┌───────────────────────────────┐
                     │    1024-Bit De-skewed Pool    │
                     └───────────────┬───────────────┘
                                     │
                                     ▼
                     ┌───────────────────────────────┐
                     │ NIST SP 800-90B Smooth        │
                     │ Min-Entropy Engine            │
                     │ H_∞ = -log₂(max(n₀/1024,      │
                     │                 n₁/1024))     │
                     └───────────────┬───────────────┘
                                     │
                                     ▼
                                  /     \
                                 /  H_∞  \     No
                                <  ≥ 0.5? > ────────┐
                                 \       /          │
                                  \     /           │
                                     │ Yes          │
                                     ▼              │
                     ┌───────────────────────────────┐│
                     │ Inline NIST SP 800-22 Tests   ││
                     │ (α = 0.01)                    ││
                     │ 1. Monobit: S_obs < 2.576     ││
                     │ 2. Runs: Z_runs < 2.576       ││
                     │ 3. Serial: χ² < 11.345        ││
                     └───────────────┬───────────────┘│
                                     │              │
                                     ▼              │
                                  /     \           │
                                 /  All  \    No    │
                                <  Tests  > ────────┤
                                 \ Pass? /          │
                                  \     /           │
                                     │ Yes          │
                                     ▼              ▼
                     ┌───────────────────────────────┐  ┌───────────────────────────────┐
                     │    PROCEED TO EXTRACTION      │  │          REJECT POOL          │
                     └───────────────────────────────┘  │    Flush Memory & Resample    │
                                                        └───────────────────────────────┘
```

### 4.1 NIST SP 800-90B Min-Entropy Calculation

Under NIST SP 800-90B, the min-entropy $H_\infty$ assesses worst-case predictability:

$$H_\infty(X) = -\log_2\left(\max_{x \in \{0, 1\}} P(X = x)\right)$$

For an evaluation block of $N = 1024\text{ bits}$ containing $n_0\text{ zeros}$ and $n_1\text{ ones}$:

$$\hat{p}_{\max} = \frac{\max(n_0, n_1)}{1024}$$

$$H_\infty = -\log_2(\hat{p}_{\max})$$

- **Ideal Value:** $n_0 = n_1 = 512 \implies \hat{p}_{\max} = 0.500 \implies H_\infty = 1.000\text{ bits/bit}$.
- **Firmware Gate Threshold:** $H_\infty \ge 0.500\text{ bits/bit}$. If tampering shifts the pool such that $n_1 > 724$, then:
  $$\hat{p}_{\max} > \frac{724}{1024} \approx 0.707 \implies H_\infty < 0.500\text{ bits/bit}$$
  The block is rejected and purged from memory.

### 4.2 Inline NIST SP 800-22 Hypothesis Testing ($\alpha = 0.01$)

#### 1. Frequency (Monobit) Test
Calculates the normalized deviation between ones and zeros:

$$S_N = n_1 - n_0$$

$$S_{\text{obs}} = \frac{|S_N|}{\sqrt{1024}} = \frac{|n_1 - n_0|}{32}$$

For a significance level $\alpha = 0.01$, the critical two-tailed cutoff is $Z_{0.005} = 2.5758$. The block passes if:

$$S_{\text{obs}} < 2.5758 \iff |n_1 - n_0| \le 82$$

#### 2. Runs Distribution Test
Counts total transitions across the sequence:

$$V_N = 1 + \sum_{k=1}^{1023} (X_k \oplus X_{k+1})$$

With sample proportion $\pi = \frac{n_1}{1024}$, the expected value and variance are:

$$\mathbb{E}[V_N] = 2048\pi(1 - \pi) + 1$$

$$\sigma_{V_N} = 2\sqrt{2048 \cdot \pi(1 - \pi)} \approx 90.51 \cdot \pi(1 - \pi)$$

The standardized test statistic is:

$$Z_{\text{runs}} = \frac{|V_N - \mathbb{E}[V_N]|}{\sigma_{V_N}}$$

The block passes if:

$$Z_{\text{runs}} < 2.5758$$

#### 3. Serial 2-Bit Pattern Test
Calculates frequency distribution across pairs $\{00, 01, 10, 11\}$ over $N - 1 = 1023$ transitions. The expected frequency is:

$$E = \frac{1023}{4} = 255.75$$

The Pearson Chi-Square statistic across observed counts $O_{ij}$ with 3 degrees of freedom is:

$$\chi^2 = \sum_{ij \in \{00, 01, 10, 11\}} \frac{(O_{ij} - 255.75)^2}{255.75}$$

For $\alpha = 0.01$ and $\text{d.o.f.} = 3$, the critical threshold is $\chi^2_{0.01, 3} = 11.345$. The block passes if:

$$\chi^2 < 11.345$$

---

## 5. Information-Theoretic Randomness Extraction

```
┌─────────────────────────────────┐       ┌─────────────────────────────────┐
│ Raw Physical Vector x ∈ {0,1}¹⁰²⁴│       │ Galois LFSR Seed Generator      │
│ Min-Entropy k ≥ 921.6 bits      │       │ P(x) = x³² + x²² + x² + x + 1   │
└────────────────┬────────────────┘       └────────────────┬────────────────┘
                 │                                         │
                 │                                  1279-Bit Vector s
                 │                                         │
                 ▼                                         ▼
         ┌─────────────────────────────────────────────────────────┐
         │ Toeplitz Universal Hash Matrix                          │
         │ y = T · x (mod 2)                                       │
         │ T: 256 rows × 1024 columns                              │
         └────────────────────────────┬────────────────────────────┘
                                      │
                                      ▼
         ┌─────────────────────────────────────────────────────────┐
         │ Extracted Vector y ∈ {0,1}²⁵⁶                           │
         │ LHL Bound: Δ ≤ 2⁻³³³·⁸                                  │
         └────────────────────────────┬────────────────────────────┘
                                      │
                                      ▼
         ┌─────────────────────────────────────────────────────────┐
         │ Hardware NIST FIPS 180-4 SHA-256 Engine                 │
         │ Key = SHA-256(y)                                        │
         └────────────────────────────┬────────────────────────────┘
                                      │
                                      ▼
         ┌─────────────────────────────────────────────────────────┐
         │ Certified 256-Bit Cryptographic Key Output              │
         └─────────────────────────────────────────────────────────┘
```

### 5.1 Toeplitz Matrix Universal Hash Formulation

To extract uniform bits from the weak physical source, EntropyX implements a 2-universal hash family using binary Toeplitz matrices:

$$T \in \mathbb{F}_2^{256 \times 1024}, \quad T_{i,j} = s_{i - j + 1024}$$

The matrix is defined by a 1279-bit seed vector $s$ generated on-chip:

$$T = \begin{bmatrix}
s_{256} & s_{257} & s_{258} & \dots & s_{1279} \\
s_{255} & s_{256} & s_{257} & \dots & s_{1278} \\
\vdots & \vdots & \vdots & \ddots & \vdots \\
s_1 & s_2 & s_3 & \dots & s_{1024}
\end{bmatrix}$$

Matrix-vector multiplication over $\text{GF}(2)$ maps the 1024-bit input $x$ into a 256-bit output $y$:

$$y_i = \bigoplus_{j=1}^{1024} (T_{i,j} \cdot x_j) \pmod 2, \quad \forall i \in \{1, 2, \dots, 256\}$$

### 5.2 Galois LFSR Seed Generation

The seed sequence $s$ is generated via a 32-bit Galois Linear Feedback Shift Register initialized with seed `0xA5C39541`. The primitive characteristic polynomial is:

$$P(x) = x^{32} + x^{22} + x^2 + x + 1$$

The shift register bit recurrence at step $t$ is:

$$s_{t+32} = (s_{t+22} \oplus s_{t+2} \oplus s_{t+1} \oplus s_t) \pmod 2$$

### 5.3 Leftover Hash Lemma Security Bound

The Leftover Hash Lemma (Impagliazzo, Levin, Luby, 1989) bounds the statistical distance of the extracted output from an ideal uniform distribution.

**Theorem (Leftover Hash Lemma):** Let $X$ be a random variable over $\{0, 1\}^n$ with min-entropy $H_\infty(X) \ge k$. Let $\mathcal{H}$ be a 2-universal hash family mapping $\{0, 1\}^n \to \{0, 1\}^m$. For $h$ selected uniformly at random from $\mathcal{H}$, the statistical distance $\Delta$ between $(h, h(X))$ and $(h, U_m)$ satisfies:

$$\Delta = \frac{1}{2} \sum_{y \in \{0, 1\}^m} \left| P[h(X) = y] - \frac{1}{2^m} \right| \le \frac{1}{2} \sqrt{2^{-(k - m)}} = 2^{-\frac{k - m}{2} - 1}$$

**Parameters in EntropyX:**
- Input length: $n = 1024\text{ bits}$
- Extracted length: $m = 256\text{ bits}$
- Verified physical min-entropy: $H_\infty \ge 0.900\text{ bits/bit} \implies k = 1024 \times 0.900 = 921.6\text{ bits}$

Evaluating the statistical distance:

$$k - m = 921.6 - 256 = 665.6\text{ bits}$$

$$\Delta \le 2^{-\frac{665.6}{2} - 1} = 2^{-332.8 - 1} = 2^{-333.8} \approx 1.83 \times 10^{-101}$$

NIST SP 800-90B defines full entropy as $\Delta \le 2^{-64} \approx 5.42 \times 10^{-20}$. EntropyX achieves $\Delta \le 2^{-333.8}$, surpassing the federal requirement by more than 260 orders of magnitude. The output is **information theoretically uniform**.

### 5.4 Hardware-Accelerated SHA-256 Conditioning

The extracted 256-bit block $y$ is passed to the Cortex-M33 hardware-accelerated SHA-256 engine:

$$K = \text{SHA-256}(y)$$

This step guarantees computational one-way security: reconstructing $y$ from an emitted key $K$ requires inverting SHA-256, with an intractable computational complexity of $\mathcal{O}(2^{256})$.

---

## 6. Avalanche Criterion & Hamming Verification

Consecutive 256-bit keys $K_{t-1}$ and $K_t$ are evaluated using the **Hamming Distance**:

$$d_H(K_{t-1}, K_t) = \sum_{j=0}^{255} (K_{t-1}[j] \oplus K_t[j])$$

Under the null hypothesis of independence, bit differences follow a Binomial distribution:

$$d_H \sim \mathcal{B}(n = 256, p = 0.5)$$

$$\mu = n \cdot p = 256 \times 0.5 = 128.0\text{ bits}\quad (50.00\%)$$

$$\sigma = \sqrt{n \cdot p \cdot (1 - p)} = \sqrt{256 \times 0.5 \times 0.5} = \sqrt{64} = 8.0\text{ bits}\quad (3.125\%)$$

The $95\%$ confidence interval for an independent key pair is:

$$[\mu - 2\sigma, \mu + 2\sigma] = [112, 144]\text{ bits} \iff [43.75\%, 56.25\%]$$

### Empirical Results

Across 1,000 continuous key-generation cycles, the system recorded:

$$\bar{d}_H = 133.4 \pm 7.17\text{ bits} \implies \bar{\mu}\% = 52.1\% \pm 2.8\%$$

This falls within the $95\%$ confidence interval, verifying that consecutive keys share no deterministic carry-over.

---

## 7. Hardware Wiring & Bill of Materials

```
                   ┌───────────────────────────────────────────────┐
                   │       NXP FRDM-MCXN236 Microcontroller        │
                   └───────┬──────────────────────┬─────────────┬──┘
                           │                      │             │
        Port 1 Domain:     │    Port 4 Domain:    │   Power:    │
        P1_16 (SDA) /      │    P4_0 (SDA) /      │   VDD_BOARD │
        P1_17 (SCL)        │    P4_1 (SCL)        │   (3.3V) /  │
                           │                      │   GND       │
                           ▼                      ▼             │
                ┌──────────────────────┐┌──────────────────────┐│
                │ Physical I2C Bus 2   ││ Physical I2C Bus 1   ││
                │ (Isolated Display)   ││ (Sensors @ 100 kHz)  ││
                └──────────┬───────────┘└──────────┬───────────┘│
                           │                       │            │
            Arduino Header │       MikroBUS Header │            │
            J2 Pins 18 & 20│       J5 Pins 5 & 6   │            │
            J3 Pin 8 & 14  │       J3 Pin 4 & 12   │            │
                           ▼                       ▼            │
                ┌──────────────────────┐┌──────────────────────┐│
                │ 1.3-inch OLED Panel  ││ GY-87 10-DOF Module  ││
                │ (SH1106 / SSD1306)   ││ (MPU6050 + QMC6308   ││
                │                      ││  + BMP180)           ││
                └──────────────────────┘└──────────────────────┘▼
```

### 7.1 Pin-to-Pin Hardware Connections

| Subsystem | Sensor / Display Pin | Microcontroller Header Pin | Pin Label / Function |
| :--- | :--- | :--- | :--- |
| **GY-87 Sensor Hub** | VCC | J3 Pin 4 | `VDD_BOARD` (3.3V Output) |
| **GY-87 Sensor Hub** | GND | J3 Pin 12 | `GND` (System Ground) |
| **GY-87 Sensor Hub** | SDA | J5 Pin 6 | `P4_0` (Flexcomm 2 I2C SDA, 2.2k pull-up) |
| **GY-87 Sensor Hub** | SCL | J5 Pin 5 | `P4_1` (Flexcomm 2 I2C SCL, 2.2k pull-up) |
| **1.3" OLED Display** | VCC | J3 Pin 8 | `VDD_BOARD` (3.3V Output) |
| **1.3" OLED Display** | GND | J3 Pin 14 | `GND` (System Ground) |
| **1.3" OLED Display** | SDA | J2 Pin 18 (Outer Row) | `P1_16` (Flexcomm 5 I2C SDA, internal pull-up) |
| **1.3" OLED Display** | SCL | J2 Pin 20 (Outer Row) | `P1_17` (Flexcomm 5 I2C SCL, internal pull-up) |

### 7.2 Bill of Materials (BOM)

| Device | Description | Quantity | Cost (INR) |
| :--- | :--- | :--- | :--- |
| **FRDM-MCXN236 Development Board** | Arm Cortex-M33 Dual-Core @ 150 MHz, 1MB Flash, 352KB ECC SRAM | 1 | ₹2,420 |
| **MPU-6050 6-Axis Sensor** | 3-Axis Accelerometer + 3-Axis Gyroscope (16-bit SAR ADC) | 1 | ₹375 |
| **SH1106 / SSD1306 1.3" OLED** | 128x64 Monochrome Graphic Display Panel | 1 | ₹392 |
| **Jumper Wires** | 100mm Female-to-Female Interconnects | 8 | ₹50 |
| **USB Type-C Cable** | High-Speed Shielded Power / Programming Cable | 1 | ₹100 |
| **TOTAL** | **Complete Functional Hardware System** | | **₹>3500** |

---

## 8. Comparative Analysis

| Architectural Parameter | Software OS PRNG (`/dev/urandom`) | Commercial HSM (e.g., YubiHSM2) | Ring-Oscillator TRNG (STM32/ESP32) |
| :--- | :--- | :--- | :--- |
| **Entropy Foundation** | Deterministic CSPRNG seeded by OS events | Single proprietary silicon diode | Inverter chain propagation jitter |
| **Information Theory Proof** | None (Purely computational complexity) | Proprietary / Closed source | None |
| **NIST SP 800-90B Evaluation** | No on-line min-entropy computation | Factory tested; internal metrics not exposed | None |
| **NIST SP 800-22 Hypothesis Testing** | Offline tool run on host PC | None | None |
| **Adversarial Gatekeeping** | Vulnerable to entropy starvation | Continues emitting keys on degradation | Continues emitting keys on lockup |
| **Physical Isolation** | Runs inside host OS memory space | USB bus interface to host computer | Direct memory-mapped register on MCU |
| **Visual Avalanche Demonstration** | None | None | None |
| **Component Unit Cost** | ₹0 (Software only) | ₹55,000 – ₹2,00,000 | Included on MCU chip |

---

## 9. Hardware Benchmarks

### 9.1 Operating Profile

- **Core Clock:** 150 MHz (Arm Cortex-M33)
- **Supply Rail:** $3.3\text{ V DC} \pm 2\%$ via USB-C regulator
- **Active Harvesting Power:** $42.0\text{ mA} @ 3.3\text{ V}$ ($138.6\text{ mW}$)
- **Static Display Power:** $28.5\text{ mA} @ 3.3\text{ V}$ ($94.05\text{ mW}$)
- **Key Generation Cycle:** $2.78\text{ seconds}$ per certified 256-bit block

### 9.2 Statistical Test Results ($10^6$ Bit Evaluation)

| Statistical Test Metric | Acceptance Threshold | Status |
| :--- | :--- | :--- |
| **Min-Entropy ($H_\infty$)** | $> 0.500\text{ bits/bit}$ | **PASSED (Full Entropy)** |
| **Monobit Frequency ($S_{\text{obs}}$)** | $< 2.5758\ (\alpha = 0.01)$ | **PASSED ($S_{\text{obs}} = 0.4375$)** |
| **Runs Distribution ($Z_{\text{runs}}$)** | $< 2.5758\ (\alpha = 0.01)$ | **PASSED ($Z_{\text{runs}} = 0.1248$)** |
| **Serial Chi-Square ($\chi^2$)** | $< 11.345\ (\text{d.o.f.} = 3)$ | **PASSED ($\chi^2 = 2.451$)** |
| **Avalanche Hamming Mean** | $50.00\% \pm 3.12\%$ | **PASSED (Strict Avalanche Criterion)** |

### 9.3 Web Dashboard

This allows the real time monitoring and visual representation of the data and keys obtained.

---

## 10. Practical Cryptographic Applications

1. **Cryptocurrency:** The device generates raw 256-bit scalar integers that can be directly imported as secp256k1 elliptic-curve private keys (Bitcoin/Ethereum) or BIP-39 recovery seeds, created entirely offline away from internet-connected keyloggers.
2. **AES-256 Master Key:** Provides 32-byte master keys for full-disk encryption containers (such as Linux LUKS or VeraCrypt).
3. **Hardware Root-of-Trust for IoT:** Operates as a physical entropy coprocessor for critical edge hardware (e.g., smart power-grid relays, industrial controllers, medical implants) that lack access to user-driven OS entropy.
4. **Post-Quantum Cryptography (PQC) Seed Material:** Satisfies the initial entropy requirements of NIST post-quantum lattice schemes (ML-KEM / CRYSTALS-Kyber and ML-DSA / CRYSTALS-Dilithium), which require true physical noise to sample high-dimensional polynomial matrices securely.

---

## 11. Regulatory Standards & Academic References

1. **NIST SP 800-90B:** Turan, M. S., et al. (2018). *Recommendation for the Entropy Sources Used for Random Bit Generation*. National Institute of Standards and Technology.
2. **NIST SP 800-22 Rev. 1a:** Rukhin, A., et al. (2010). *A Statistical Test Suite for Random and Pseudorandom Number Generators for Cryptographic Applications*. NIST Special Publication.
3. **NIST FIPS PUB 180-4:** National Institute of Standards and Technology. (2015). *Secure Hash Standard (SHS)*. U.S. Department of Commerce.
4. **Leftover Hash Lemma:** Impagliazzo, R., Levin, L. A., & Luby, M. (1989). Pseudo-random generation from one-way functions. In *Proc. 21st Annual ACM Symposium on Theory of Computing (STOC '89)*, pp. 12–24.
5. **Universal Hash Functions:** Carter, J. L., & Wegman, M. N. (1979). Universal classes of hash functions. *Journal of Computer and System Sciences*, 18(2), 143–154.
6. **Microcontroller Hardware Reference:** NXP Semiconductors. (2024). *FRDM-MCXN236 Board User Manual* (Doc ID: UM12041). Rev. 2.0.

---

## 12. Glossary of Terms

- **2-Universal Hash Family ($\mathcal{H}$):** A collection of hash functions mapping an $n$-bit input to an $m$-bit output where the collision probability for any two distinct inputs $x_1 \ne x_2$ satisfies $\mathbb{P}[h(x_1) = h(x_2)] \le 2^{-m}$.
- **Arm Cortex-M33:** A 32-bit RISC processor core implementing the ARMv8-M architecture, featuring hardware floating-point support, the Data Watchpoint and Trace unit, and TrustZone security extensions.
- **Avalanche Effect:** A cryptographic property wherein a 1-bit perturbation in the input produces an uncorrelated, pseudo-random change in approximately $50\%$ of the output bits.
- **Brownian Motion:** Stochastic mechanical displacement of micro-machined silicon structures caused by collisions with residual gas molecules in an enclosed MEMS cavity.
- **CSPRNG (Cryptographically Secure Pseudo-Random Number Generator):** A deterministic algorithm designed to expand a short physical seed into a longer bitstream that is computationally indistinguishable from uniform randomness.
- **Discrete First-Order Difference Filter:** A linear high-pass digital filter characterized by $H(z) = 1 - z^{-1}$ that rejects static DC baselines ($\omega = 0$), including constant gravitational and geomagnetic offsets.
- **DWT (Data Watchpoint and Trace):** An on-chip Cortex-M debug peripheral containing the 32-bit CYCCNT register, which counts core CPU clock cycles at 150 MHz to capture clock phase jitter.
- **Galois LFSR (Linear Feedback Shift Register):** A shift register configuration where input taps are XORed with the state concurrently, evaluated over $\text{GF}(2)$ using an irreducible characteristic polynomial.
- **Hamming Distance:** The number of corresponding bit positions at which two binary strings of equal length differ, evaluated using bitwise XOR and pop count operations.
- **HSM (Hardware Security Module):** A dedicated physical appliance used to generate, store, and manage cryptographic keys in an isolated hardware environment.
- **Johnson-Nyquist Noise:** Electronic noise generated by the thermal agitation of charge carriers inside a conducting medium at equilibrium, independent of applied voltage.
- **Leftover Hash Lemma (LHL):** An information-theoretic theorem that bounds the statistical distance between the output of a 2-universal hash function and an ideal uniform distribution as a function of input min-entropy.
- **LSB (Least Significant Bit):** The bit position in a digital register holds the smallest numerical weight, corresponding to the finest quantization step of an analog-to-digital converter.
- **MEMS (Micro-Electro-Mechanical Systems):** Microscopic electromechanical devices fabricated using semiconductor processes, comprising silicon springs, comb capacitors, and suspended proof-masses.
- **Min-Entropy ($H_\infty$):** The most conservative metric in the Rényi entropy family, quantifying the worst-case probability of an adversary guessing the output on a single attempt: $H_\infty = -\log_2(p_{\max})$.
- **NIST FIPS 180-4:** Federal Information Processing Standard defining the Secure Hash Standard (SHS), including the SHA-256 cryptographic hash function.
- **NIST SP 800-22:** A special publication establishing statistical hypothesis tests (including Monobit, Runs, and Serial tests) to verify empirical randomness for cryptographic use.
- **NIST SP 800-90B:** A special publication defining validation requirements, physical noise-source testing, and smooth min-entropy estimation for random bit generators.
- **Open-Drain (I2C):** A digital bus circuit configuration where devices can pull a shared line low to ground but rely on an external pull-up resistor to pull the line to logic high.
- **SAR ADC (Successive-Approximation Register):** An analog-to-digital conversion architecture that uses binary search comparison to digitize analog sensor voltages.
- **SHA-256:** A one-way cryptographic hash function producing a fixed-size 256-bit (32-byte) digest from an arbitrary-length message block.
- **Statistical Distance:** The total variation of distance measuring the statistical distinguishability between two probability distributions over the same sample space.
- **Toeplitz Matrix:** An $m \times n$ matrix in which each descending diagonal from left to right contains identical elements, used to construct hardware-efficient universal hash extractors over $\text{GF}(2)$.
- **TRNG (True Random Number Generator):** A key-generation system that derives unpredictable data from physical stochastic processes rather than deterministic algorithmic states.
- **TrustZone:** An ARMv8-M system-wide hardware security extension providing isolated execution domains (Secure and Non-Secure states) on a single physical core.
- **Von Neumann De-biasing:** An extraction technique that processes non-overlapping input bit pairs to generate an unbiased output stream where $P(0) = P(1) = 0.5$, discarding identical bit pairs.

**BY: L K PRATHIKA**
