import { useEffect, useState } from "react"
import { useParams } from "react-router-dom"
import { MarkdownRenderer } from "../components/MarkdownRenderer"

// Mapping of route slugs to actual filenames in public/docs
const routeMap: Record<string, string> = {
    "getting-started": "01_introduction.md",
    "introduction": "01_introduction.md",
    "installation": "02_installation.md",
    "hello-world": "03_hello_world.md",
    "variables": "04_variables.md",
    "data-types": "05_data_types.md",
    "operators": "06_operators.md",
    "conditionals": "07_conditionals.md",
    "loops": "08_loops.md",
    "functions": "09_functions.md",
    "builtins": "10_builtins.md",
    "lists": "11_lists.md",
    "dictionaries": "12_dictionaries.md",
    "keywords": "13_keywords.md",
    "errors": "14_errors.md",
}

export function DocPage() {
    const { slug } = useParams()
    const [content, setContent] = useState("")
    const [loading, setLoading] = useState(true)

    useEffect(() => {
        async function loadContent() {
            setLoading(true)
            try {
                const route = slug || "introduction"
                const filename = routeMap[route] || "01_introduction.md"

                const response = await fetch(`/docs/${filename}`)
                if (!response.ok) throw new Error("Failed to load")
                const text = await response.text()
                setContent(text)
            } catch (error) {
                console.error(error)
                setContent("# Error loading documentation")
            } finally {
                setLoading(false)
            }
        }

        loadContent()
    }, [slug])

    if (loading) {
        return <div className="flex h-[50vh] items-center justify-center">Loading...</div>
    }

    return (
        <div className="pb-16 pt-6 md:pb-24 lg:pb-32">
            <MarkdownRenderer content={content} />
        </div>
    )
}
