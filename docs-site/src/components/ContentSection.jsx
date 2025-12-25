import React from 'react';

const ContentSection = ({ id, title, children }) => (
    <section id={id} className="mb-20 scroll-mt-24">
        <h2 className="text-3xl font-bold mb-6 text-zinc-100 flex items-center group">
            {title}
            <a href={`#${id}`} className="ml-2 text-zinc-700 opacity-0 group-hover:opacity-100 transition-opacity hover:text-blue-400">#</a>
        </h2>
        <div className="prose prose-invert prose-zinc max-w-none">
            {children}
        </div>
    </section>
);

export default ContentSection;
