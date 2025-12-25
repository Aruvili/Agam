import React, { useState, useEffect } from 'react';
import { motion, AnimatePresence } from 'framer-motion';
import { ChevronRight, Menu, X, Search } from 'lucide-react';
import { cn } from '../lib/utils';

const sidebarData = [
    {
        title: "Getting Started",
        items: [
            { title: "Introduction", slug: "introduction" },
            { title: "Installation", slug: "installation" },
            { title: "Hello World", slug: "hello-world" }
        ]
    },
    {
        title: "Basics",
        items: [
            { title: "Variables", slug: "variables" },
            { title: "Data Types", slug: "data-types" },
            { title: "Operators", slug: "operators" }
        ]
    },
    {
        title: "Control Flow",
        items: [
            { title: "Conditionals", slug: "conditionals" },
            { title: "Loops", slug: "loops" }
        ]
    },
    {
        title: "Functions",
        items: [
            { title: "Functions", slug: "functions" },
            { title: "Built-in Functions", slug: "built-in-functions" }
        ]
    },
    {
        title: "Data Structures",
        items: [
            { title: "Lists", slug: "lists" },
            { title: "Dictionaries", slug: "dictionaries" }
        ]
    },
    {
        title: "Reference",
        items: [
            { title: "Keywords", slug: "keywords" },
            { title: "Error Messages", slug: "error-messages" }
        ]
    },
    {
        title: "Advanced Features",
        items: [
            { title: "Structs", slug: "structs" },
            { title: "Enums", slug: "enums" },
            { title: "Pattern Matching", slug: "pattern-matching" },
            { title: "Error Handling", slug: "error-handling" },
            { title: "Modules", slug: "modules" },
            { title: "File I/O", slug: "file-io" }
        ]
    }
];

export default function Sidebar({ className }) {
    const [activeSlug, setActiveSlug] = useState('introduction');
    const [openSections, setOpenSections] = useState({});
    const [isMobileOpen, setIsMobileOpen] = useState(false);
    const [searchQuery, setSearchQuery] = useState('');

    const filteredSidebarData = React.useMemo(() => {
        if (!searchQuery) return sidebarData;

        return sidebarData.map(section => {
            const filteredItems = section.items.filter(item =>
                item.title.toLowerCase().includes(searchQuery.toLowerCase())
            );
            return {
                ...section,
                items: filteredItems
            };
        }).filter(section => section.items.length > 0);
    }, [searchQuery]);

    // Auto-expand sections when searching
    useEffect(() => {
        if (searchQuery) {
            const allOpen = {};
            filteredSidebarData.forEach(section => {
                allOpen[section.title] = true;
            });
            setOpenSections(allOpen);
        }
    }, [searchQuery, filteredSidebarData]);

    // Initialize all sections as open by default or based on logic
    useEffect(() => {
        const initialOpen = {};
        sidebarData.forEach(section => {
            initialOpen[section.title] = true;
        });
        setOpenSections(initialOpen);
    }, []);

    const toggleSection = (title) => {
        setOpenSections(prev => ({
            ...prev,
            [title]: !prev[title]
        }));
    };

    // Scroll Spy logic
    useEffect(() => {
        const handleScroll = () => {
            const sections = sidebarData.flatMap(s => s.items.map(i => i.slug));
            for (const slug of sections) {
                const element = document.getElementById(slug);
                if (element) {
                    const rect = element.getBoundingClientRect();
                    if (rect.top >= 0 && rect.top < 300) {
                        setActiveSlug(slug);
                        break;
                    }
                }
            }
        };
        window.addEventListener('scroll', handleScroll);
        return () => window.removeEventListener('scroll', handleScroll);
    }, []);

    const handleLinkClick = (slug) => {
        setActiveSlug(slug);
        setIsMobileOpen(false);
        const element = document.getElementById(slug);
        if (element) {
            element.scrollIntoView({ behavior: 'smooth' });
        }
    };

    // Mobile overlay
    useEffect(() => {
        if (isMobileOpen) {
            document.body.style.overflow = 'hidden';
        } else {
            document.body.style.overflow = 'unset';
        }
        return () => { document.body.style.overflow = 'unset'; }
    }, [isMobileOpen]);

    return (
        <>

            <button
                onClick={() => setIsMobileOpen(true)}
                className="lg:hidden fixed top-4 left-4 z-50 p-2 bg-zinc-900 text-zinc-400 rounded-md border border-zinc-800"
            >
                <Menu size={20} />
            </button>


            <AnimatePresence>
                {isMobileOpen && (
                    <motion.div
                        initial={{ opacity: 0 }}
                        animate={{ opacity: 1 }}
                        exit={{ opacity: 0 }}
                        onClick={() => setIsMobileOpen(false)}
                        className="fixed inset-0 bg-black/60 backdrop-blur-sm z-40 lg:hidden"
                    />
                )}
            </AnimatePresence>


            <motion.aside
                className={cn(
                    "fixed top-0 left-0 bottom-0 z-50 w-64 bg-zinc-950 border-r border-zinc-800 overflow-hidden flex flex-col",
                    "lg:translate-x-0 transition-transform duration-300 ease-in-out",
                    isMobileOpen ? "translate-x-0" : "-translate-x-full",
                    className
                )}
            >

                <div className="p-6 border-b border-zinc-900">
                    <div className="flex items-center justify-between mb-4">
                        <h1 className="text-xl font-bold bg-gradient-to-r from-blue-400 to-purple-400 bg-clip-text text-transparent">
                            அகம் (Agam)
                        </h1>
                        <button
                            onClick={() => setIsMobileOpen(false)}
                            className="lg:hidden text-zinc-500 hover:text-zinc-300"
                        >
                            <X size={20} />
                        </button>
                    </div>

                    <div className="relative group">
                        <Search className="absolute left-3 top-2.5 text-zinc-500 group-focus-within:text-blue-400 transition-colors" size={16} />
                        <input
                            value={searchQuery}
                            onChange={(e) => setSearchQuery(e.target.value)}
                            placeholder="Search..."
                            className="w-full bg-zinc-900 border border-zinc-800 text-sm text-zinc-300 rounded-md pl-9 pr-4 py-2 focus:outline-none focus:border-blue-500/50 focus:ring-1 focus:ring-blue-500/50 transition-all placeholder:text-zinc-600"
                        />
                    </div>
                </div>


                <div className="flex-1 overflow-y-auto py-6 px-4 space-y-6 scrollbar-thin scrollbar-thumb-zinc-800 scrollbar-track-transparent">
                    {filteredSidebarData.map((section) => (
                        <div key={section.title} className="space-y-2">
                            <button
                                onClick={() => toggleSection(section.title)}
                                className="flex items-center w-full text-xs font-semibold uppercase tracking-wider text-zinc-500 hover:text-zinc-300 transition-colors group"
                            >
                                <motion.span
                                    animate={{ rotate: openSections[section.title] ? 90 : 0 }}
                                    transition={{ duration: 0.2 }}
                                    className="mr-2"
                                >
                                    <ChevronRight size={12} />
                                </motion.span>
                                {section.title}
                            </button>

                            <AnimatePresence initial={false}>
                                {openSections[section.title] && (
                                    <motion.div
                                        initial={{ height: 0, opacity: 0 }}
                                        animate={{ height: 'auto', opacity: 1 }}
                                        exit={{ height: 0, opacity: 0 }}
                                        transition={{ duration: 0.3, ease: "easeInOut" }}
                                        className="overflow-hidden"
                                    >
                                        <ul className="space-y-1 pl-2 border-l border-zinc-800/50 ml-1.5">
                                            {section.items.map((item) => (
                                                <li key={item.slug}>
                                                    <button
                                                        onClick={() => handleLinkClick(item.slug)}
                                                        className={cn(
                                                            "w-full text-left px-4 py-2 text-sm rounded-md transition-all duration-200 relative overflow-hidden",
                                                            activeSlug === item.slug
                                                                ? "text-blue-400 font-medium bg-blue-500/10"
                                                                : "text-zinc-400 hover:text-zinc-200 hover:bg-zinc-900/50"
                                                        )}
                                                    >
                                                        {activeSlug === item.slug && (
                                                            <motion.div
                                                                layoutId="active-nav-indicator"
                                                                className="absolute inset-0 border-l-2 border-blue-400 bg-gradient-to-r from-blue-500/10 to-transparent opacity-50"
                                                                initial={{ opacity: 0 }}
                                                                animate={{ opacity: 1 }}
                                                                exit={{ opacity: 0 }}
                                                            />
                                                        )}
                                                        <span className="relative z-10">{item.title}</span>
                                                    </button>
                                                </li>
                                            ))}
                                        </ul>
                                    </motion.div>
                                )}
                            </AnimatePresence>
                        </div>
                    ))}
                </div>

            
                <div className="p-4 border-t border-zinc-900 text-xs text-zinc-600 text-center">
                    <p>v0.1.1 Agam docs</p>
                </div>
            </motion.aside>
        </>
    );
}
