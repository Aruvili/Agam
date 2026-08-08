# 21. தொகுப்பு மேலாளர் (Agam Package Manager - agamp)

`agamp` என்பது அகம் (Agam) நிரலாக்க மொழிக்கான அதிகாரப்பூர்வத் தொகுப்பு மேலாளர் (Package Manager) ஆகும். 

---

## முக்கியக் கட்டளைகள் (CLI Commands)

```bash
# 1. புதிய திட்டம் உருவாக்குதல் (Create New Project)
agamp new my_project

# 2. திட்டப்பொறி தொகுத்தல் (Compile Project)
agamp build
agamp build --release

# 3. திட்டப்பொறி இயக்குதல் (Run Project)
agamp run

# 4. பிழை சோதனை செய்தல் (Type-Check Only)
agamp check

# 5. தொகுப்பு சேர்த்தல் (Add Package Dependency)
agamp add valaiccevaiyagam
agamp add https://github.com/user/custom_pkg.git

# 6. தொகுப்புகளைப் புதுப்பித்தல் (Update Installed Packages)
agamp update

# 7. இலக்குக் கோப்பகத்தைச் சுத்தம் செய்தல் (Clean Target Directory)
agamp clean
```

---

## தொகுப்புத் தெளிவாக்க வரிசை (Package Resolution Pipeline)

1. **மையப் பதிவேடு (Central Package Registry - `registry/index.json`)**
2. **உள்ளூர் தரநிலைத் தொகுப்புகள் (Standard Library - `std/packages/`)**
3. **Git தொலைநிலைப் பதிவேடு (Git Remote Repository Fallback)**

---

## Lockfile (`pk.lock`)

`agamp add` கட்டளையை இயக்கும் போது, திட்டக் கோப்பகத்தில் `pk.lock` என்ற பூட்டுக்கோப்பு (lockfile) தானாகவே உருவாக்கப்பட்டுப் பராமரிக்கப்படும்.
