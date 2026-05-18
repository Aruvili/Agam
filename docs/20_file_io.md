# Chapter 20: File I/O (கோப்பு படிக்க/எழுத)

## Overview

File I/O (Input/Output) allows your programs to read from and write to files. In agam, you can use built-in functions for file operations.

---

## File Functions

| Tamil | English | Purpose |
|-------|---------|---------|
| `படி` | `read_file` | Read file contents |
| `எழுது` | `write_file` | Write to file |
| `உள்ளது` | `file_exists` | Check if file exists |

---

## Reading Files

### படி / read_file

Read the entire contents of a file:

```
மாறி content = படி("data.txt");
பதிப்பி(content);
```

### Reading Line by Line

```
மாறி content = படி("data.txt");
மாறி lines = பிரி(content, "\n");

சுற்று (line உள் lines) {
    பதிப்பி(line);
}
```

### Safe File Reading

Always check if a file exists before reading:

```
எனில் உள்ளது("config.txt"):
    மாறி config = படி("config.txt");
    பதிப்பி("Config loaded!");
} இல்லையெனில் {
    பதிப்பி("Config file not found!");
    மாறி config = ""  # Default value;
}
```

---

## Writing Files

### எழுது / write_file

Write content to a file (creates or overwrites):

```
மாறி content = "வணக்கம் உலகம்!";
எழுது("output.txt", content)
பதிப்பி("File written successfully!");
```

### Writing Multiple Lines

```
மாறி lines = ["Line 1", "Line 2", "Line 3"];
மாறி content = இணை("\n", lines);
எழுது("output.txt", content)
```

### Appending to Files

Read existing content and add new content:

```
செயல் append_to_file(filename, new_content) {
    மாறி existing = "";
    
    எனில் உள்ளது(filename):
        existing = படி(filename);
    
    எழுது(filename, existing + "\n" + new_content)

}
append_to_file("log.txt", "New log entry")
```

---

## Checking File Existence

### உள்ளது / file_exists

Check if a file exists:

```
எனில் உள்ளது("important.txt"):
    பதிப்பி("File exists!")
இல்லை:
    பதிப்பி("File not found!")
```

---

## Practical Examples

### Example 1: Log File

```
செயல் log(message) {
    மாறி timestamp = "2024-01-15"  # In real app, get current time;
    மாறி entry = timestamp + ": " + message + "\n";
    
    மாறி existing = "";
    எனில் உள்ளது("app.log"):
        existing = படி("app.log");
    
    எழுது("app.log", existing + entry)

}
log("Application started")
log("User logged in")
log("Data processed")
```

### Example 2: Configuration File

```
# config.txt:
# username=admin
# theme=dark
# language=tamil

செயல் load_config(filename) {
    மாறி config = {}
    
    எனில் உள்ளது(filename):
        மாறி content = படி(filename);
        மாறி lines = பிரி(content, "\n");
        
        சுற்று (line உள் lines) {
            எனில் உள்ளதா(line, "="):;
                மாறி parts = பிரி(line, "=");
                மாறி key = parts[0];
                மாறி value = parts[1];
                config[key] = value;
    
    }
    விடை config;

}
செயல் save_config(filename, config) {
    மாறி lines = [];
    # Iterate through config and build lines
    எழுது(filename, இணை("\n", lines))

}
மாறி settings = load_config("config.txt");
பதிப்பி("Username:", settings["username"]);
```

### Example 3: CSV File Reader

```
செயல் read_csv(filename) {
    எனில் இல்ல உள்ளது(filename):
        வீசு("CSV file not found: " + filename)
    
    மாறி content = படி(filename);
    மாறி lines = பிரி(content, "\n");
    மாறி data = [];
    
    சுற்று (line உள் lines) {
        எனில் நீளம்(ஒழுங்கு(line)) > 0:
            மாறி row = பிரி(line, ",");
            சேர்(data, row)
    
    }
    விடை data;

}
# data.csv:
# name,age,city
# Raja,25,Chennai
# Priya,30,Mumbai

மாறி records = read_csv("data.csv");
சுற்று (row உள் records) {
    பதிப்பி(row);
}
```

### Example 4: Simple Database

```
அமைப்பு Contact {
    name,
    phone,
    email,

}
செயல் save_contacts(filename, contacts) {
    மாறி lines = [];
    சுற்று (contact உள் contacts) {
        மாறி line = contact.name + "|" + contact.phone + "|" + contact.email;
        சேர்(lines, line),
    
    }
    எழுது(filename, இணை("\n", lines)),
    பதிப்பி("Saved", நீளம்(contacts), "contacts"),

}
செயல் load_contacts(filename) {
    மாறி contacts = [];
    
    எனில் உள்ளது(filename):,
        மாறி content = படி(filename);
        மாறி lines = பிரி(content, "\n");
        
        சுற்று (line உள் lines) {
            எனில் நீளம்(ஒழுங்கு(line)) > 0:,
                மாறி parts = பிரி(line, "|");
                மாறி contact = Contact(parts[0], parts[1], parts[2]);
                சேர்(contacts, contact),
    
    }
    விடை contacts,

}
# Usage
மாறி contacts = [;
    Contact("Raja", "9876543210", "raja@email.com"),
    Contact("Priya", "9876543211", "priya@email.com"),
],

save_contacts("contacts.db", contacts),

# Later...
மாறி loaded = load_contacts("contacts.db");
சுற்று (c உள் loaded) {
    பதிப்பி(c.name, "-", c.phone),
}
```

### Example 5: Text File Processor

```
செயல் process_file(input_file, output_file) {
    எனில் இல்ல உள்ளது(input_file):
        பதிப்பி("Input file not found!");
        விடை
    
    மாறி content = படி(input_file);
    
    # Process: uppercase and remove empty lines
    மாறி lines = பிரி(content, "\n");
    மாறி processed = [];
    
    சுற்று (line உள் lines) {
        மாறி trimmed = ஒழுங்கு(line);
        எனில் நீளம்(trimmed) > 0:
            சேர்(processed, மேல்(trimmed))
    
    }
    எழுது(output_file, இணை("\n", processed))
    பதிப்பி("Processed", நீளம்(processed), "lines");

}
process_file("input.txt", "output.txt")
```

---

## Error Handling with Files

Always handle potential file errors:

```
முயற்சி {
    மாறி content = படி("important_data.txt");
    # Process content
} பிடி (error) {
    பதிப்பி("Error reading file:", error);
    # Use default or fallback
}
```

---

## File Path Tips

```
# Current directory
படி("file.txt")

# Subdirectory
படி("data/file.txt")

# Absolute path (Windows)
படி("C:/Users/Name/file.txt")

# Absolute path (Linux/Mac)
படி("/home/user/file.txt")
```

---

## Best Practices

1. **Always check if files exist** before reading
2. **Handle errors gracefully** with try-catch
3. **Close/clean up** after file operations
4. **Use appropriate paths** for your OS
5. **Don't read huge files** into memory at once

---

## Summary

- Use `படி` (read_file) to read file contents
- Use `எழுது` (write_file) to write to files
- Use `உள்ளது` (file_exists) to check file existence
- Always handle errors and check for file existence
- Combine with string functions for parsing file data

---

**Previous: [Chapter 19: Modules ←](19_modules.md)**

---

*Congratulations! You've completed the Agam documentation. Happy coding in Tamil! 🎉*
