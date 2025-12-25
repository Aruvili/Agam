import React from 'react';

const SkeletonDoc = () => {
    return (
        <div className="min-h-screen bg-zinc-950 text-zinc-50 font-sans flex">
            {/* Skeleton Sidebar */}
            <aside className="hidden lg:flex flex-col w-64 fixed top-0 bottom-0 border-r border-zinc-800 bg-zinc-950 p-6 z-20">
                <div className="h-8 w-32 bg-zinc-900 rounded mb-8 animate-pulse" />
                <div className="space-y-6">
                    {[...Array(5)].map((_, i) => (
                        <div key={i} className="space-y-3">
                            <div className="h-4 w-24 bg-zinc-900 rounded animate-pulse" />
                            <div className="space-y-2 pl-3 border-l border-zinc-800">
                                <div className="h-3 w-full bg-zinc-900/50 rounded animate-pulse" />
                                <div className="h-3 w-3/4 bg-zinc-900/50 rounded animate-pulse" />
                            </div>
                        </div>
                    ))}
                </div>
            </aside>

            {/* Skeleton Main Content */}
            <main className="lg:pl-64 w-full">
                <div className="max-w-4xl mx-auto px-6 py-12 lg:py-16">
                    {/* Hero/Header Skeleton */}
                    <div className="mb-12 space-y-4">
                        <div className="h-10 w-3/4 bg-zinc-900 rounded animate-pulse" />
                        <div className="h-4 w-full bg-zinc-900/50 rounded animate-pulse" />
                        <div className="h-4 w-5/6 bg-zinc-900/50 rounded animate-pulse" />
                    </div>

                    {/* Content Section Skeleton */}
                    {[...Array(3)].map((_, sectionIndex) => (
                        <div key={sectionIndex} className="mb-16 space-y-6">
                            {/* Section Title */}
                            <div className="flex items-center gap-4 mb-6">
                                <div className="h-8 w-48 bg-zinc-900 rounded animate-pulse" />
                            </div>

                            {/* Paragraph lines */}
                            <div className="space-y-3">
                                <div className="h-4 w-full bg-zinc-900/50 rounded animate-pulse" />
                                <div className="h-4 w-11/12 bg-zinc-900/50 rounded animate-pulse" />
                                <div className="h-4 w-4/5 bg-zinc-900/50 rounded animate-pulse" />
                            </div>

                            {/* Code Block Skeleton */}
                            <div className="h-32 w-full bg-zinc-900/30 border border-zinc-900 rounded-lg animate-pulse" />

                            {/* Table Skeleton */}
                            <div className="border border-zinc-900 rounded-lg overflow-hidden">
                                <div className="h-10 bg-zinc-900/50 animate-pulse border-b border-zinc-900" />
                                {[...Array(3)].map((j) => (
                                    <div key={j} className="h-12 bg-zinc-900/20 animate-pulse border-b border-zinc-900/50 last:border-0" />
                                ))}
                            </div>
                        </div>
                    ))}
                </div>
            </main>
        </div>
    );
};

export default SkeletonDoc;
