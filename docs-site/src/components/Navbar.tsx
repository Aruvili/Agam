import { Link } from "react-router-dom"
import { Github } from "lucide-react"
import { Button } from "./ui/button"
import { ModeToggle } from "./mode-toggle"
import { cn } from "../lib/utils"

export function Navbar() {
    return (
        <header className="sticky top-0 z-50 w-full border-b border-border/40 bg-background/95 backdrop-blur supports-[backdrop-filter]:bg-background/60">
            <div className="container flex h-14 max-w-screen-2xl items-center">
                <div className="mr-4 hidden md:flex">
                    <Link to="/" className="mr-6 flex items-center space-x-2">
                        <span className="hidden font-bold sm:inline-block">
                            அகம் (Agam)
                        </span>
                    </Link>
                    <nav className="flex items-center gap-6 text-sm">
                        <Link to="/docs" className="transition-colors hover:text-foreground/80 text-foreground/60">Documentation</Link>
                        <Link to="/docs/components" className="transition-colors hover:text-foreground/80 text-foreground/60">Reference</Link>
                    </nav>
                </div>
                <div className="flex flex-1 items-center justify-between space-x-2 md:justify-end">
                    <div className="w-full flex-1 md:w-auto md:flex-none">
                        <Button
                            variant="outline"
                            className={cn(
                                "relative h-8 w-full justify-start rounded-[0.5rem] bg-background text-sm font-normal text-muted-foreground shadow-none sm:pr-12 md:w-40 lg:w-64"
                            )}
                        >
                            <span className="hidden lg:inline-flex">Search documentation...</span>
                            <span className="inline-flex lg:hidden">Search...</span>
                            <kbd className="pointer-events-none absolute right-[0.3rem] top-[0.3rem] hidden h-5 select-none items-center gap-1 rounded border bg-muted px-1.5 font-mono text-[10px] font-medium opacity-100 sm:flex">
                                <span className="text-xs">⌘</span>K
                            </kbd>
                        </Button>
                    </div>
                    <nav className="flex items-center">
                        <Link to="https://github.com/aruvili/agam" target="_blank" rel="noreferrer">
                            <div className={cn("inline-flex h-9 w-9 items-center justify-center rounded-md font-medium transition-colors hover:bg-accent hover:text-accent-foreground focus-visible:outline-none focus-visible:ring-1 focus-visible:ring-ring disabled:pointer-events-none disabled:opacity-50")}>
                                <Github className="h-4 w-4" />
                                <span className="sr-only">GitHub</span>
                            </div>
                        </Link>
                        <ModeToggle />
                    </nav>
                </div>
            </div>
        </header>
    )
}
