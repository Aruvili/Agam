# மாற்ற பதிவு - Changelog

அனைத்து குறிப்பிடத்தக்க மாற்றங்களும் இந்த கோப்பில் ஆவணப்படுத்தப்படும்.

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.0] - 2025-12-23

### Added - சேர்க்கப்பட்டவை

#### Core Language Features - மொழி அம்சங்கள்
- **Variables** - மாறிகள்: `மாறி`, `மாறாத` (let, const)
- **Data Types** - தரவு வகைகள்: Numbers, Strings, Booleans, Lists, Dictionaries
- **Operators** - அளவீடுகள்: Arithmetic, Comparison, Logical
- **Control Flow** - கட்டுப்பாடு: `என்றால்`/`இல்லையென்றால்`/`இல்லை` (if/elif/else)
- **Loops** - வளையங்கள்: `வரை` (while), `ஒவ்வொரு` (for)
- **Functions** - செயல்கள்: `செயல்` (fn) with recursion support
- **Structs** - கட்டமைப்பு: User-defined data structures
- **Enums** - விருப்பம்: Enumeration types
- **Pattern Matching** - பொருத்து: Match expressions with wildcard support
- **Error Handling** - பிழை கையாளுதல்: `முயற்சி`/`பிடி`/`வீசு` (try/catch/throw)
- **Module System** - இறக்குமதி: Import external modules

#### Built-in Functions - உள்ளமைந்த செயல்கள்
- **Math** - கணிதம்: `வர்க்கம்`, `அடி`, `தளம்`, `கூரை`, `முழுமை`, `குறைந்தபட்சம்`, `அதிகபட்சம்`, `தற்செயல்`, `கூட்டு`
- **String** - சரம்: `மேல்`, `கீழ்`, `ஒழுங்கு`, `பிரி`, `இணை`, `மாற்று`, `தொடங்கு`, `முடிவு`, `உள்ளதா`
- **List** - பட்டியல்: `நீளம்`, `சேர்`, `நீக்கு`, `வரிசை`, `தலைகீழ்`
- **Type** - வகை: `வகை`, `எண்ணாக`, `தசமாக`, `சரமாக`
- **I/O** - உள்ளீடு/வெளியீடு: `அச்சிடு`, `உள்ளீடு`
- **File** - கோப்பு: `படி`, `எழுது`, `உள்ளது`

#### Security - பாதுகாப்பு
- Recursion depth limit (1000 calls)
- Loop iteration limit (10 million iterations)
- File read size limit (10 MB)
- Range size limit (1 million elements)

#### Bilingual Support - இருமொழி ஆதரவு
- Full Tamil and English keyword support
- Tamil error messages with English translations

#### Documentation - ஆவணங்கள்
- 14-chapter learning book in `docs/`
- Example programs in `examples/`
- Marketing website in `website/`

### Technical Details - தொழில்நுட்ப விவரங்கள்
- **Runtime**: Tree-walking interpreter
- **Parser**: Recursive descent parser
- **Lexer**: Hand-written scanner with Unicode support
- **Memory**: Reference-counted values with interior mutability

---

## Future Plans - எதிர்கால திட்டங்கள்

### [0.2.0] - Planned
- Lambda/anonymous functions
- String interpolation
- Standard library modules (time, http)
- REPL improvements

### [0.3.0] - Planned
- Bytecode compilation
- Performance optimizations
- Package manager
