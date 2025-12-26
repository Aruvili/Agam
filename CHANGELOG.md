# மாற்ற பதிவு - Changelog

அனைத்து குறிப்பிடத்தக்க மாற்றங்களும் இந்த கோப்பில் ஆவணப்படுத்தப்படும்.

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.3] - 2025-12-26

### Performance - செயல்திறன் மேம்பாடுகள்
- Performance Optimized

### Dependencies - புதிய சார்புகள்
- Added `rustc-hash` , `smallvec` , `once_cell` , `phf` 

---

## [0.1.2] - 2025-12-25

### Fixed - சரிசெய்தவை
- **Input function** - `உள்ளீடு()` / `input()` now works correctly in expressions
  - Parser now properly handles the `Ulleedu` token as a callable identifier

### Added - சேர்க்கப்பட்டவை

#### REPL Improvements - REPL மேம்பாடுகள்
- **Line numbers** - Each prompt shows `[n] >>>` with line count
- **Expression results** - Expression values now displayed with `=> result`
- **Clear screen** - `clear()` / `அழி()` / `cls` to clear the terminal
- **Command history** - `history()` / `வரலாறு()` to view previous commands
- **Simpler commands** - `exit`, `help`, `?` work without parentheses
- **Better multi-line** - Improved block detection with tab support
- **EOF handling** - Graceful exit on Ctrl+D with farewell message
- **Enhanced help** - Comprehensive help with examples in a nice box format

#### Lambda/Anonymous Functions - செயலி செயல்கள்
- **Tamil syntax** - `செயலி(x, y): x + y`
- **English syntax** - `lambda(x, y): x + y`
- **Arrow syntax** - `(x, y) => x + y`
- Lambdas can be assigned to variables and called like regular functions
- Support for closures (captures enclosing scope)

#### String Interpolation - சரம் இணைப்பு
- **F-String syntax** - `f"Hello {name}!"` or `f"வணக்கம் {பெயர்}!"`
- Embed any expression inside `{}` braces
- Supports variable references, function calls, and complex expressions
- Example: `f"2 + 3 = {2 + 3}"` → `"2 + 3 = 5"`
- Escape braces with `{{` and `}}`

#### Time Module - நேரம் தொகுதி
- `நேரம்()` / `time()` - Get Unix timestamp in seconds
- `தூக்கம்(n)` / `sleep(n)` - Pause execution for n seconds
- `தேதி()` / `date()` - Get formatted date string (YYYY-MM-DD HH:MM:SS)
- `நாள்()` / `now()` - Get current time as dictionary with components:
  - `year`/`ஆண்டு`, `month`/`மாதம்`, `day`/`நாள்`
  - `hour`/`மணி`, `minute`/`நிமிடம்`, `second`/`வினாடி`

#### HTTP & JSON Module - வலை தொகுதி
- `வலை_படி(url)` / `http_get(url)` - HTTP GET request, returns `{status, body}`
- `வலை_அனுப்பு(url, data)` / `http_post(url, data)` - HTTP POST request
- `வலை_புதுப்பி(url, data)` / `http_put(url, data)` - HTTP PUT request
- `வலை_நீக்கு(url)` / `http_delete(url)` - HTTP DELETE request
- `கோப்பு_பதிவேற்று(url, path)` / `file_upload(url, path)` - Upload file via multipart/form-data
- `வலை_கோரிக்கை(config)` / `http_request(config)` - Flexible HTTP with custom headers
  - Config: `{url, method, headers: {...}, body}`
  - Supports GET, POST, PUT, DELETE, PATCH, HEAD methods
- `json_படி(str)` / `json_parse(str)` - Parse JSON string into Agam value
- Supports nested objects, arrays, strings, numbers, booleans, null

#### WebSocket Module - சாக்கெட் தொகுதி
- `சாக்கெட்_இணை(url)` / `ws_connect(url)` - Connect to WebSocket server, returns `{id, status, connected}`
- `சாக்கெட்_அனுப்பு(conn, msg)` / `ws_send(conn, msg)` - Send message through WebSocket
- `சாக்கெட்_படி(conn)` / `ws_receive(conn)` - Receive message (blocking)
- `சாக்கெட்_மூடு(conn)` / `ws_close(conn)` - Close WebSocket connection

---

## [0.1.1] - 2025-12-24

### Fixed - மேற்கொண்டவை
- List, Dictionary, import methods (issue #1)
- Documentation (issue #2)
- minor typos
- minor bugs fixed

---

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
- Async/await
- More string methods

### [0.3.0] - Planned
- Bytecode compilation
- Performance optimizations
- Package manager
