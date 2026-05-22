# Power section

The power path feeds the **servo rail (5 V BEC)** and the **logic rail (3.3 V)** from the battery input. A slide switch and **high-side PMOS** enable the high-current 5 V path; an **LDO** derives 3.3 V for the MCU and sensors.

| Block | Role |
| ----- | ---- |
| Slide switch | Main input enable |
| High-side PMOS + soft-start (C14) | Switched 5 V to servos; limits inrush |
| LDO | 5 V → 3.3 V for MCU and sensors |
| Bulk / decoupling | Rail stability on 5 V and 3.3 V |
| Servo headers | High-current outputs on the BEC path |

## Schematic

![Power schematic](../assets/power-schematic.png)

## Design — high-side PMOS and soft-start

A **high-side PMOS** switches the board on and off. Five hobby servos on the **5 V** rail draw large **stall currents**, and bulk capacitance on that rail produces a high **inrush** if the FET turns on instantly.

A **100 nF soft-start capacitor (C14)** between gate and drain slows the turn-on and limits inrush. The FET is sized for roughly **750 mA** per servo stall; design headroom targets **~5 A** steady capability with inrush limited to about **10 A for 1 ms**, in line with the device datasheet. At the intended gate drive, **R<sub>DS(on)</sub>** is on the order of **10 mΩ**, so conduction loss stays acceptable even during short high-current events.

## Design — LDO (3.3 V)

*(Brief note on LDO choice, input range, and expected load — MCU + sensors on 3.3 V.)*

---

## Simulation (LTspice)

Pre-layout checks in LTspice; schematics and `.asc` files can live under `hardware/` or a linked archive when added.

| Plot | File (add under `docs/assets/`) | What it should show |
| ---- | ------------------------------- | ------------------- |
| Soft-start / inrush | `power-soft-start-sim.png` | Gate drive and 5 V rail during enable |
| PMOS switch / load | `power-pmos-load-sim.png` | Steady-state or switched load on 5 V |
| LDO transient *(optional)* | `power-ldo-load-sim.png` | 3.3 V during load step on logic rail |

### Soft-start and inrush

<!-- Replace with your LTspice plot when ready:
<p align="center">
  <img src="../assets/power-soft-start-sim.png" alt="LTspice soft-start simulation">
  <br>
  <sub>LTspice — soft-start / inrush (conditions: …)</sub>
</p>
-->

*Plot pending.*

**Takeaways**

- *(e.g. Peak inrush below 10 A for 1 ms with C14 = 100 nF.)*
- *(e.g. 5 V rail reaches regulation within … ms.)*

### PMOS switch under load

*Plot pending.*

**Takeaways**

- *(Steady R<sub>DS(on)</sub> loss vs. expected servo stall current.)*

### LDO load step *(optional)*

*Plot pending.*

**Takeaways**

- *(3.3 V droop and recovery with MCU + sensor load model.)*

---

## Bench measurements (oscilloscope)

Measurements on the **v1.0** board during bring-up. Record **probe point**, **scope settings**, and **load** (bench supply vs. battery, servos connected or not) in each caption so plots stay comparable later.

| Measurement | File (add under `docs/assets/`) | Typical probe / condition |
| ----------- | ------------------------------- | ------------------------- |
| Enable / inrush | `power-inrush-scope.png` | 5 V rail or FET drain; slide switch on |
| 5 V rail steady | `power-5v-scope.png` | BEC rail under load |
| 3.3 V rail | `power-3v3-scope.png` | LDO output; logic load |

### Enable and inrush

<!-- Example figure block (match physical-design.md / README caption style):
<p align="center">
  <img src="../assets/power-inrush-scope.png" alt="Oscilloscope capture — inrush">
  <br>
  <sub>Scope — 5 V enable (CH1: …, CH2: …, load: …)</sub>
</p>
-->

*Capture pending.*

**Observations**

- *(Does measured inrush match simulation within …?)*
- *(Any overshoot or ringing worth noting?)*

### 5 V rail (servo / BEC)

*Capture pending.*

### 3.3 V rail (LDO)

*Capture pending.*

---

## Simulation vs. measurement

Short comparison after both sets of plots are in place:

| Check | Simulation | Measured | Pass? |
| ----- | ---------- | -------- | ----- |
| Peak inrush | | | |
| 5 V regulation | | | |
| 3.3 V regulation | | | |

---

## Summary

- **Switching:** PMOS + C14 soft-start limits inrush while supporting multi-servo stall current.
- **Simulation:** *(one line — what LTspice confirmed.)*
- **Bench:** *(one line — what scope measurements confirmed on v1.0.)*

---

**Next:** [Storage →](storage.md)

[Documentation index](../index.md)
