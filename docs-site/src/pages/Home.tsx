import { Link } from "react-router-dom"
import { Button } from "../components/ui/button"

export function Home() {
    return (
        <div className="flex flex-col min-h-screen">
            <section className="space-y-6 pb-8 pt-6 md:pb-12 md:pt-10 lg:py-32">
                <div className="container flex max-w-[64rem] flex-col items-center gap-4 text-center">
                    <Link
                        to="/docs/introduction"
                        className="rounded-2xl bg-muted px-4 py-1.5 text-sm font-medium"
                        target="_blank"
                    >
                        v0.1.0 is here! <span className="font-bold text-primary">Read the changelog</span>
                    </Link>
                    <h1 className="font-heading text-3xl sm:text-5xl md:text-6xl lg:text-7xl font-bold bg-clip-text text-transparent bg-gradient-to-r from-teal-500 to-primary pb-2">
                        Program in your <br /> mother tongue.
                    </h1>
                    <p className="max-w-[42rem] leading-normal text-muted-foreground sm:text-xl sm:leading-8">
                        அகம் (Agam) is a modern, statically typed programming language built with Rust.
                        Write code in Tamil with English compatibility. Fast, Safe, and Expressive.
                    </p>
                    <div className="space-x-4">
                        <Link to="/docs/getting-started">
                            <Button size="lg" className="h-11 px-8">
                                Get Started
                            </Button>
                        </Link>
                        <Link to="https://github.com/aruvili/agam" target="_blank" rel="noreferrer">
                            <Button size="lg" variant="outline" className="h-11 px-8">
                                GitHub
                            </Button>
                        </Link>
                    </div>
                </div>
            </section>

            <section className="container space-y-6 bg-slate-50 py-8 dark:bg-transparent md:py-12 lg:py-24">
                <div className="mx-auto flex max-w-[58rem] flex-col items-center space-y-4 text-center">
                    <h2 className="font-heading text-3xl leading-[1.1] sm:text-3xl md:text-6xl font-bold">
                        தமிழில் நிரலாக்கம்
                    </h2>
                    <p className="max-w-[85%] leading-normal text-muted-foreground sm:text-lg sm:leading-7">
                        (Programming in Tamil)
                    </p>
                </div>
                <div className="mx-auto grid justify-center gap-4 sm:grid-cols-2 md:max-w-[64rem] md:grid-cols-3">
                    <div className="relative overflow-hidden rounded-lg border bg-background p-2">
                        <div className="flex h-[180px] flex-col justify-between rounded-md p-6">
                            <div className="space-y-2">
                                <h3 className="font-bold">Tamil Syntax</h3>
                                <p className="text-sm text-muted-foreground">
                                    First-class support for Tamil keywords like `மாறி`, `என்றால்`, and `வரை`.
                                </p>
                            </div>
                        </div>
                    </div>
                    <div className="relative overflow-hidden rounded-lg border bg-background p-2">
                        <div className="flex h-[180px] flex-col justify-between rounded-md p-6">
                            <div className="space-y-2">
                                <h3 className="font-bold bg-clip-text text-transparent bg-gradient-to-r from-orange-400 to-red-600">Rust Speed</h3>
                                <p className="text-sm text-muted-foreground">
                                    Built on Rust for blazing fast performance and memory safety without GC pauses.
                                </p>
                            </div>
                        </div>
                    </div>
                    <div className="relative overflow-hidden rounded-lg border bg-background p-2">
                        <div className="flex h-[180px] flex-col justify-between rounded-md p-6">
                            <div className="space-y-2">
                                <h3 className="font-bold">Bilingual</h3>
                                <p className="text-sm text-muted-foreground">
                                    Mix Tamil and English keywords seamlessly. Perfect for education and transition.
                                </p>
                            </div>
                        </div>
                    </div>
                </div>
            </section>

            <section className="container py-8 md:py-12 lg:py-24">
                <div className="mx-auto flex max-w-[58rem] flex-col items-center justify-center gap-4 text-center">
                    <h2 className="font-heading text-3xl leading-[1.1] sm:text-3xl md:text-6xl font-bold">
                        Example
                    </h2>
                    <div className="w-full max-w-2xl rounded-lg border shadow-lg overflow-hidden text-left bg-[#1e1e1e]">
                        <div className="flex items-center gap-1.5 px-4 py-2 border-b border-white/10 bg-white/5">
                            <div className="w-3 h-3 rounded-full bg-red-500" />
                            <div className="w-3 h-3 rounded-full bg-yellow-500" />
                            <div className="w-3 h-3 rounded-full bg-green-500" />
                        </div>
                        <pre className="p-4 text-sm font-mono text-gray-300 overflow-x-auto">
                            <code>
                                <span className="text-[#569cd6]">செயல்</span> <span className="text-[#dcdcaa]">வணக்கம்</span>(பெயர்):{'\n'}
                                {'    '}<span className="text-[#c586c0]">திரும்பு</span> <span className="text-[#ce9178]">"வணக்கம், "</span> + பெயர் + <span className="text-[#ce9178]">"!"</span>{'\n'}
                                {'\n'}
                                <span className="text-[#dcdcaa]">அச்சிடு</span>(<span className="text-[#dcdcaa]">வணக்கம்</span>(<span className="text-[#ce9178]">"உலகம்"</span>))
                            </code>
                        </pre>
                    </div>
                </div>
            </section>

        </div>
    )
}
