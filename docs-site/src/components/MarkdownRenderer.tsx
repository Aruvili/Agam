import ReactMarkdown from 'react-markdown'
import { Prism as SyntaxHighlighter } from 'react-syntax-highlighter'
import { vscDarkPlus } from 'react-syntax-highlighter/dist/esm/styles/prism'
import rehypeRaw from 'rehype-raw'
import { cn } from "../lib/utils"

interface MarkdownRendererProps {
    content: string
}

export function MarkdownRenderer({ content }: MarkdownRendererProps) {
    return (
        <div className="prose prose-slate dark:prose-invert max-w-none">
            <ReactMarkdown
                rehypePlugins={[rehypeRaw]}
                components={{
                    code({ node, inline, className, children, ...props }: any) {
                        const match = /language-(\w+)/.exec(className || '')
                        return !inline && match ? (
                            <div className="relative rounded-md overflow-hidden my-4">
                                <div className="absolute right-2 top-2 z-10">
                                    {/* Copy button could go here */}
                                </div>
                                <SyntaxHighlighter
                                    {...props}
                                    style={vscDarkPlus}
                                    language={match[1]}
                                    PreTag="div"
                                    customStyle={{ margin: 0, borderRadius: '0.5rem', fontSize: '0.9rem' }}
                                >
                                    {String(children).replace(/\n$/, '')}
                                </SyntaxHighlighter>
                            </div>
                        ) : (
                            <code {...props} className={cn("bg-muted px-1.5 py-0.5 rounded font-mono text-sm", className)}>
                                {children}
                            </code>
                        )
                    },
                    h1: ({ className, ...props }: any) => (
                        <h1 className={cn("mt-2 scroll-m-20 text-4xl font-bold tracking-tight", className)} {...props} />
                    ),
                    h2: ({ className, ...props }: any) => (
                        <h2 className={cn("mt-10 scroll-m-20 border-b pb-1 text-3xl font-semibold tracking-tight first:mt-0", className)} {...props} />
                    ),
                    h3: ({ className, ...props }: any) => (
                        <h3 className={cn("mt-8 scroll-m-20 text-2xl font-semibold tracking-tight", className)} {...props} />
                    ),
                    h4: ({ className, ...props }: any) => (
                        <h4 className={cn("mt-8 scroll-m-20 text-xl font-semibold tracking-tight", className)} {...props} />
                    ),
                    p: ({ className, ...props }: any) => (
                        <p className={cn("leading-7 [&:not(:first-child)]:mt-6", className)} {...props} />
                    ),
                    ul: ({ className, ...props }: any) => (
                        <ul className={cn("my-6 ml-6 list-disc [&>li]:mt-2", className)} {...props} />
                    ),
                    blockquote: ({ className, ...props }: any) => (
                        <blockquote className={cn("mt-6 border-l-2 pl-6 italic text-muted-foreground", className)} {...props} />
                    ),
                    table: ({ className, ...props }: any) => (
                        <div className="my-6 w-full overflow-y-auto">
                            <table className={cn("w-full", className)} {...props} />
                        </div>
                    ),
                    tr: ({ className, ...props }: any) => (
                        <tr className={cn("m-0 border-t p-0 even:bg-muted", className)} {...props} />
                    ),
                    th: ({ className, ...props }: any) => (
                        <th className={cn("border px-4 py-2 text-left font-bold [&[align=center]]:text-center [&[align=right]]:text-right", className)} {...props} />
                    ),
                    td: ({ className, ...props }: any) => (
                        <td className={cn("border px-4 py-2 text-left [&[align=center]]:text-center [&[align=right]]:text-right", className)} {...props} />
                    ),
                }}
            >
                {content}
            </ReactMarkdown>
        </div>
    )
}
