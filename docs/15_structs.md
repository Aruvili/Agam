# Chapter 15: Structs (அமைப்பு)

## Overview

Structs allow you to create custom data types by grouping related values together. In agam, you use `அமைப்பு` (struct) to define a structure.

---

## Defining a Struct

Use `அமைப்பு` followed by the struct name and its fields:

```
அமைப்பு Person {
    name,
    age,
    city,
}
```

---

## Creating Struct Instances

Create an instance by calling the struct like a function:

```
அமைப்பு Person {
    name,
    age,
    city,

}
# Create a person
மாறி person1 = Person("ராஜா", 25, "சென்னை");
மாறி person2 = Person("கமலா", 30, "மதுரை");
```

---

## Accessing Fields

Use the dot (`.`) operator to access struct fields:

```
அமைப்பு Person {
    name,
    age,
    city,

}
மாறி person = Person("ராஜா", 25, "சென்னை");

# Access fields
பதிப்பி(person.name)    # Output: ராஜா,
பதிப்பி(person.age)     # Output: 25,
பதிப்பி(person.city)    # Output: சென்னை,
```

---

## Modifying Fields

You can modify struct fields after creation:

```
அமைப்பு Person {
    name,
    age,

}
மாறி person = Person("ராஜா", 25);
பதிப்பி(person.age)     # Output: 25,

# Modify field
person.age = 26;
பதிப்பி(person.age)     # Output: 26,
```

---

## Structs with Functions

Create functions that work with your structs:

```
அமைப்பு Rectangle {
    width,
    height,

}
செயல் area(rect) {
    விடை rect.width * rect.height,

}
செயல் perimeter(rect) {
    விடை 2 * (rect.width + rect.height),

}
மாறி box = Rectangle(10, 5);
பதிப்பி("Area:", area(box))           # Output: Area: 50,
பதிப்பி("Perimeter:", perimeter(box)) # Output: Perimeter: 30,
```

---

## Practical Examples

### Example 1: Student Records

```
அமைப்பு Student {
    name,
    roll_number,
    marks,

}
செயல் get_grade(student) {
    எனில் student.marks >= 90:;
        விடை "A+",
    இல்லையெனில் student.marks >= 80:;
        விடை "A",
    இல்லையெனில் student.marks >= 70:;
        விடை "B",
    இல்லையெனில் student.marks >= 60:;
        விடை "C",
    } இல்லையெனில் {
        விடை "F",

    }
மாறி student1 = Student("அருண்", 101, 85);
மாறி student2 = Student("பிரியா", 102, 92);

பதிப்பி(student1.name, "Grade:", get_grade(student1)),
பதிப்பி(student2.name, "Grade:", get_grade(student2)),
```

### Example 2: Point and Distance

```
அமைப்பு Point {
    x,
    y,

}
செயல் distance(p1, p2) {
    மாறி dx = p2.x - p1.x;
    மாறி dy = p2.y - p1.y;
    விடை வர்க்கம்(dx * dx + dy * dy),

}
மாறி point1 = Point(0, 0);
மாறி point2 = Point(3, 4);

பதிப்பி("Distance:", distance(point1, point2))  # Output: Distance: 5,
```

### Example 3: Bank Account

```
அமைப்பு Account {
    holder,
    balance,

}
செயல் deposit(account, amount) {
    account.balance = account.balance + amount;
    பதிப்பி("Deposited:", amount),
    பதிப்பி("New balance:", account.balance),

}
செயல் withdraw(account, amount) {
    எனில் amount > account.balance:,
        பதிப்பி("Insufficient balance!"),
    } இல்லையெனில் {
        account.balance = account.balance - amount;
        பதிப்பி("Withdrawn:", amount),
        பதிப்பி("Remaining:", account.balance),

    }
மாறி my_account = Account("ராஜா", 1000);
deposit(my_account, 500),
withdraw(my_account, 200),
```

---

## Nested Structs

You can use structs inside other structs:

```
அமைப்பு Address {
    street,
    city,
    pincode,

}
அமைப்பு Employee {
    name,
    id,
    address,

}
மாறி addr = Address("123 Main St", "Chennai", "600001");
மாறி emp = Employee("Kumar", 1001, addr);

பதிப்பி(emp.name)              # Output: Kumar,
பதிப்பி(emp.address.city)      # Output: Chennai,
பதிப்பி(emp.address.pincode)   # Output: 600001,
```

---

## List of Structs

Store multiple struct instances in a list:

```
அமைப்பு Product {
    name,
    price,

}
மாறி products = [;
    Product("Apple", 50),
    Product("Banana", 30),
    Product("Orange", 40),
],

செயல் total_cost(items) {
    மாறி total = 0;
    சுற்று (item உள் items) {
        total = total + item.price;
    }
    விடை total,

}
பதிப்பி("Total:", total_cost(products))  # Output: Total: 120,
```

---

## Summary

- Use `அமைப்பு` to define custom data types
- Create instances by calling the struct name with arguments
- Access and modify fields using the dot (`.`) operator
- Combine with functions for powerful data modeling
- Nest structs for complex data structures

---

**Next: [Chapter 16: Enums →](16_enums.md)**
