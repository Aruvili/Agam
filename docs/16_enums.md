# Chapter 16: Enums (பட்டியல்)

## Overview

Enums (enumerations) allow you to define a type with a fixed set of possible values. In agam, you use `பட்டியல்` (enum) to define an enumeration.

---

## Defining an Enum

Use `பட்டியல்` followed by the enum name and its variants:

```
பட்டியல் Color {
    Red,
    Green,
    Blue,
}
```

---

## Using Enum Values

Access enum variants using the dot (`.`) operator:

```
பட்டியல் Color {
    Red,
    Green,
    Blue,

}
மாறி favorite = Color.Red;
மாறி secondary = Color.Blue;

பதிப்பி(favorite)    # Output: Color.Red,
```

---

## Comparing Enum Values

Compare enum values using `==`:

```
பட்டியல் Status {
    Active,
    Inactive,
    Pending,

}
மாறி status = Status.Active;

எனில் status == Status.Active:;
    பதிப்பி("System is active!"),
இல்லையெனில் status == Status.Pending:;
    பதிப்பி("System is pending..."),
} இல்லையெனில் {
    பதிப்பி("System is inactive."),
}
```

---

## Practical Examples

### Example 1: Days of the Week

```
பட்டியல் Day {
    Monday,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday,
    Sunday,

}
செயல் is_weekend(day) {
    எனில் day == Day.Saturday அல்லது day == Day.Sunday:;
        விடை உண்மை,
    விடை பொய்,

}
மாறி today = Day.Saturday;
எனில் is_weekend(today):,
    பதிப்பி("It's the weekend! 🎉"),
} இல்லையெனில் {
    பதிப்பி("It's a workday."),
}
```

### Example 2: Traffic Light

```
பட்டியல் TrafficLight {
    Red,
    Yellow,
    Green,

}
செயல் action(light) {
    எனில் light == TrafficLight.Red:;
        விடை "Stop!",
    இல்லையெனில் light == TrafficLight.Yellow:;
        விடை "Slow down...",
    } இல்லையெனில் {
        விடை "Go!",

    }
மாறி current = TrafficLight.Green;
பதிப்பி(action(current))  # Output: Go!,
```

### Example 3: Order Status

```
பட்டியல் OrderStatus {
    Placed,
    Processing,
    Shipped,
    Delivered,
    Cancelled,

}
அமைப்பு Order {
    id,
    product,
    status,

}
செயல் update_status(order, new_status) {
    order.status = new_status;
    பதிப்பி("Order", order.id, "is now:", new_status),

}
மாறி order = Order(1001, "Laptop", OrderStatus.Placed);
update_status(order, OrderStatus.Processing),
update_status(order, OrderStatus.Shipped),
update_status(order, OrderStatus.Delivered),
```

### Example 4: Direction

```
பட்டியல் Direction {
    North,
    South,
    East,
    West,

}
அமைப்பு Position {
    x,
    y,

}
செயல் move(pos, direction) {
    எனில் direction == Direction.North:;
        pos.y = pos.y + 1;
    இல்லையெனில் direction == Direction.South:;
        pos.y = pos.y - 1;
    இல்லையெனில் direction == Direction.East:;
        pos.x = pos.x + 1;
    } இல்லையெனில் {
        pos.x = pos.x - 1;

    }
மாறி player = Position(0, 0);
move(player, Direction.North),
move(player, Direction.East),
பதிப்பி("Position:", player.x, player.y)  # Output: Position: 1 1,
```

---

## Enums with Pattern Matching

Use `பொருத்து` (match) for cleaner enum handling:

```
பட்டியல் Season {
    Spring,
    Summer,
    Autumn,
    Winter,

}
செயல் describe(season) {
    பொருத்து (season) {
        Season.Spring => விடை "Flowers bloom 🌸";
        Season.Summer => விடை "Hot and sunny ☀️";
        Season.Autumn => விடை "Leaves fall 🍂";
        Season.Winter => விடை "Cold and snowy ❄️";

    }
மாறி current = Season.Winter;
பதிப்பி(describe(current))  # Output: Cold and snowy ❄️,
```

---

## When to Use Enums

Enums are useful when you have:

- A **fixed set of options** (colors, days, statuses)
- **State machines** (order status, game states)
- **Configuration options** (log levels, modes)
- **Type-safe alternatives** to string constants

---

## Summary

- Use `பட்டியல்` to define enumerations
- Access variants with dot notation: `EnumName.Variant`
- Compare using `==`
- Combine with `பொருத்து` for clean pattern matching
- Great for representing fixed sets of values

---

**Next: [Chapter 17: Pattern Matching →](17_pattern_matching.md)**
