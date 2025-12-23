export type NavItem = {
    title: string
    href: string
    items?: NavItem[]
}

export const docsConfig: { sidebarNav: NavItem[] } = {
    sidebarNav: [
        {
            title: "Getting Started",
            href: "/docs/getting-started",
            items: [
                { title: "Introduction", href: "/docs/introduction" },
                { title: "Installation", href: "/docs/installation" },
                { title: "Hello World", href: "/docs/hello-world" },
            ],
        },
        {
            title: "Basics",
            href: "/docs/basics",
            items: [
                { title: "Variables", href: "/docs/variables" },
                { title: "Data Types", href: "/docs/data-types" },
                { title: "Operators", href: "/docs/operators" },
            ],
        },
        {
            title: "Control Flow",
            href: "/docs/control-flow",
            items: [
                { title: "Conditionals", href: "/docs/conditionals" },
                { title: "Loops", href: "/docs/loops" },
            ],
        },
        {
            title: "Functions",
            href: "/docs/functions",
            items: [
                { title: "Defining Functions", href: "/docs/functions" },
                { title: "Built-in Functions", href: "/docs/builtins" },
            ],
        },
        {
            title: "Data Structures",
            href: "/docs/data-structures",
            items: [
                { title: "Lists", href: "/docs/lists" },
                { title: "Dictionaries", href: "/docs/dictionaries" },
            ],
        },
        {
            title: "Reference",
            href: "/docs/reference",
            items: [
                { title: "Keywords", href: "/docs/keywords" },
                { title: "Error Messages", href: "/docs/errors" },
            ],
        },
    ],
}
