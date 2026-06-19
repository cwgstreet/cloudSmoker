# cloudSmoker2 Hardware Reference

This directory serves as the root hardware repository for the **cloudSmoker2** platform. It contains mechanical housing documents, sensor calibration resources, manufacturing documentation, and the complete electronic design suite.

---

## 📂 Repository Structure

```text
├── KiCad9/                          # Core ECAD design environment (KiCad 9.x)
│   ├── cloudSmoker_ver2f.kicad_pro  # Main project configuration
│   ├── cloudSmoker_ver2f.kicad_sch  # 4-Layer Schematic blueprints
│   ├── cloudSmoker_ver2f.kicad_pcb  # Mixed-signal PCB layout 
│   ├── sym-lib-table & fp-lib-table # Local dependency resolution tables
│   ├── JLCImport.pretty/            # Component footprints (.kicad_mod)
│   ├── JLCImport.3dshapes/          # 3D structural clearance models (.step / .wrl)
│   ├── production/                  # Manufacturing artifacts (Gerbers, BOM, CPL)
│   └── svg_files/                   # Dynamic high-resolution hardware dashboard graphics
│
├── Thermistor datasheet/            # High-temperature physical probe reference material
│   ├── ET-732 (Thermistor) datasheet scan.pdf
│   └── Maverick ET-732 - 733 (Semitec 105GT-2 Thermistor) datasheet.pdf
│
├── arduino_housing_01_160717_CAD.PDF.pdf   # Mechanical chassis reference dimensions (cloudSmoker1)
└── licence.md                               # Open-source hardware licensing framework
```

## ⚙️ Key Subsystems & Artifacts

### 1. Electronic Design Automation (`KiCad9/`)
* **ECAD Engine**: Engineered exclusively inside **KiCad 9**. Backward-compatibility issues can occur if parsed with older software builds.
* **Component Portability**: Component symbols, tracking pads, and 3D packages are completely containerized locally inside the `JLCImport` folders. External global library paths are not required to audit the board.
* **Fabrication Files**: The `KiCad9/production/` subdirectory houses pre-compiled fabrication packages including the zipped Gerber layer arrays, coordinates tables (`positions.csv`), and structural parts listings (`bom.csv`).

### 2. Physical & Enclosure Overlays
* **`arduino_housing_01_160717_CAD.PDF.pdf`**: Legacy cloudSmoker1 mechanical profile drawing used to verify PCB perimeter cuts, mounting hole positions, and connector face alignments against existing chassis designs.

### 3. Thermmister Data
* **`Thermistor datasheet/`**: Houses structural lookup sheets for the Semitec 105GT-2 high-temp food probes.
---

