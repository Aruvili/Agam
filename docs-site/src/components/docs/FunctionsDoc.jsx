import React from 'react';
import ContentSection from '../ContentSection';

const FunctionsDoc = () => {
    return (
        <>
            <ContentSection id="functions" title="Functions">
                <h3 className="text-xl font-semibold text-zinc-100 mb-4">What is a Function?</h3>
                <p className="text-zinc-300 mb-4">
                    A <strong>function</strong> is a reusable block of code that performs a specific task.
                </p>

                <h3 className="text-xl font-semibold text-zinc-100 mb-4">Defining Functions</h3>
                <div className="bg-zinc-900 p-4 rounded-lg border border-zinc-800 font-mono text-sm mb-6">
                    <span className="text-purple-400">செயல்</span> greet(name):<br />
                    &nbsp;&nbsp;&nbsp;&nbsp;<span className="text-purple-400">திரும்பு</span> <span className="text-green-300">"Wannakum, "</span> + name<br /><br />
                    <span className="text-blue-400">அச்சிடு</span>(greet(<span className="text-green-300">"Agam"</span>))
                </div>

                <h3 className="text-xl font-semibold text-zinc-100 mb-4">Key Concepts</h3>
                <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
                    <div className="bg-zinc-900/50 p-4 rounded-lg border border-zinc-800">
                        <h4 className="text-white font-medium mb-1">Parameters (அளவுருக்கள்)</h4>
                        <p className="text-xs text-zinc-400 mb-2">Inputs for your function.</p>
                        <code className="text-xs font-mono text-zinc-300">
                            <span className="text-purple-400">செயல்</span> add(a, b):
                        </code>
                    </div>
                    <div className="bg-zinc-900/50 p-4 rounded-lg border border-zinc-800">
                        <h4 className="text-white font-medium mb-1">Return Values (திரும்பு)</h4>
                        <p className="text-xs text-zinc-400 mb-2">Outputs from your function.</p>
                        <code className="text-xs font-mono text-zinc-300">
                            <span className="text-purple-400">திரும்பு</span> result
                        </code>
                    </div>
                </div>
            </ContentSection>

            <ContentSection id="built-in-functions" title="Built-in Functions">
                <p className="text-zinc-300 mb-6">
                    A rich library of utility functions is provided out of the box.
                </p>

                <div className="overflow-hidden rounded-lg border border-zinc-800 bg-zinc-900/50">
                    <table className="w-full text-left text-sm text-zinc-400">
                        <thead className="bg-zinc-900 text-zinc-200">
                            <tr>
                                <th className="px-4 py-2 font-medium">Category</th>
                                <th className="px-4 py-2 font-medium">Functions</th>
                            </tr>
                        </thead>
                        <tbody className="divide-y divide-zinc-800">
                            <tr><td className="px-4 py-2 text-zinc-300">I/O</td><td className="px-4 py-2 font-mono"><span className="text-blue-400">அச்சிடு</span> (print), <span className="text-blue-400">உள்ளீடு</span> (input)</td></tr>
                            <tr><td className="px-4 py-2 text-zinc-300">Conversion</td><td className="px-4 py-2 font-mono"><span className="text-blue-400">எண்ணாக</span> (int), <span className="text-blue-400">சரமாக</span> (str)</td></tr>
                            <tr><td className="px-4 py-2 text-zinc-300">Collections</td><td className="px-4 py-2 font-mono"><span className="text-blue-400">நீளம்</span> (len), <span className="text-blue-400">சேர்</span> (append), <span className="text-blue-400">நீக்கு</span> (pop)</td></tr>
                            <tr><td className="px-4 py-2 text-zinc-300">Math</td><td className="px-4 py-2 font-mono"><span className="text-blue-400">வர்க்கம்</span> (sqrt), <span className="text-blue-400">கூட்டு</span> (sum), <span className="text-blue-400">குறைந்தபட்சம்</span> (min)</td></tr>
                        </tbody>
                    </table>
                </div>
            </ContentSection>
        </>
    );
};

export default FunctionsDoc;
