import React from 'react';
import ContentSection from '../ContentSection';

const Reference = () => {
    return (
        <>
            <ContentSection id="keywords" title="Keywords">
                <h3 className="text-xl font-semibold text-zinc-100 mb-4">Declaration Keywords</h3>
                <div className="overflow-hidden rounded-lg border border-zinc-800 bg-zinc-900/50 mb-8">
                    <table className="w-full text-left text-sm text-zinc-400">
                        <thead className="bg-zinc-900 text-zinc-200">
                            <tr><th className="px-4 py-2 font-medium">Tamil</th><th className="px-4 py-2 font-medium">English</th><th className="px-4 py-2 font-medium">Description</th></tr>
                        </thead>
                        <tbody className="divide-y divide-zinc-800">
                            <tr><td className="px-4 py-2 font-mono text-purple-300">செயல்</td><td className="px-4 py-2 font-mono">fn</td><td className="px-4 py-2">Define a function</td></tr>
                            <tr><td className="px-4 py-2 font-mono text-purple-300">மாறி</td><td className="px-4 py-2 font-mono">let</td><td className="px-4 py-2">Declare a variable</td></tr>
                            <tr><td className="px-4 py-2 font-mono text-purple-300">மாறாத</td><td className="px-4 py-2 font-mono">const</td><td className="px-4 py-2">Declare a constant</td></tr>
                        </tbody>
                    </table>
                </div>

                <h3 className="text-xl font-semibold text-zinc-100 mb-4">Control Flow Keywords</h3>
                <div className="overflow-hidden rounded-lg border border-zinc-800 bg-zinc-900/50 mb-8">
                    <table className="w-full text-left text-sm text-zinc-400">
                        <thead className="bg-zinc-900 text-zinc-200">
                            <tr><th className="px-4 py-2 font-medium">Tamil</th><th className="px-4 py-2 font-medium">English</th><th className="px-4 py-2 font-medium">Description</th></tr>
                        </thead>
                        <tbody className="divide-y divide-zinc-800">
                            <tr><td className="px-4 py-2 font-mono text-purple-300">என்றால்</td><td className="px-4 py-2 font-mono">if</td><td className="px-4 py-2">If condition</td></tr>
                            <tr><td className="px-4 py-2 font-mono text-purple-300">இல்லையென்றால்</td><td className="px-4 py-2 font-mono">elif</td><td className="px-4 py-2">Else if</td></tr>
                            <tr><td className="px-4 py-2 font-mono text-purple-300">இல்லை</td><td className="px-4 py-2 font-mono">else</td><td className="px-4 py-2">Else</td></tr>
                            <tr><td className="px-4 py-2 font-mono text-purple-300">வரை</td><td className="px-4 py-2 font-mono">while</td><td className="px-4 py-2">While loop</td></tr>
                            <tr><td className="px-4 py-2 font-mono text-purple-300">ஒவ்வொரு</td><td className="px-4 py-2 font-mono">for</td><td className="px-4 py-2">For loop</td></tr>
                            <tr><td className="px-4 py-2 font-mono text-purple-300">உள்ள</td><td className="px-4 py-2 font-mono">in</td><td className="px-4 py-2">In operator</td></tr>
                            <tr><td className="px-4 py-2 font-mono text-purple-300">திரும்பு</td><td className="px-4 py-2 font-mono">return</td><td className="px-4 py-2">Return value</td></tr>
                            <tr><td className="px-4 py-2 font-mono text-purple-300">நிறுத்து</td><td className="px-4 py-2 font-mono">break</td><td className="px-4 py-2">Break loop</td></tr>
                            <tr><td className="px-4 py-2 font-mono text-purple-300">தொடர்</td><td className="px-4 py-2 font-mono">continue</td><td className="px-4 py-2">Continue loop</td></tr>
                        </tbody>
                    </table>
                </div>
            </ContentSection>

            <ContentSection id="error-messages" title="Error Messages">
                <h3 className="text-xl font-semibold text-zinc-100 mb-4">Common Errors</h3>
                <div className="grid gap-4">
                    <div className="bg-zinc-900/50 p-4 rounded-lg border border-zinc-800">
                        <h4 className="text-red-400 font-medium mb-1">வரையறுக்கப்படாத மாறி (Undefined Variable)</h4>
                        <p className="text-sm text-zinc-400 mb-2">Error: You're using a variable that doesn't exist.</p>
                        <code className="text-xs font-mono text-zinc-500">
                            <span className="text-blue-400">அச்சிடு</span>(பெயர்)   <span className="text-red-500">Error: பெயர் is not defined</span>
                        </code>
                    </div>
                    <div className="bg-zinc-900/50 p-4 rounded-lg border border-zinc-800">
                        <h4 className="text-red-400 font-medium mb-1">பூஜ்ஜியத்தால் வகுக்க இயலாது (Division by Zero)</h4>
                        <p className="text-sm text-zinc-400 mb-2">Error: Cannot divide by zero.</p>
                        <code className="text-xs font-mono text-zinc-500">
                            <span className="text-purple-400">மாறி</span> result = 10 / 0   <span className="text-red-500">Error!</span>
                        </code>
                    </div>
                    <div className="bg-zinc-900/50 p-4 rounded-lg border border-zinc-800">
                        <h4 className="text-red-400 font-medium mb-1">':' எதிர்பார்க்கப்படுகிறது (Expected ':')</h4>
                        <p className="text-sm text-zinc-400 mb-2">Error: Missing colon after condition or function.</p>
                        <code className="text-xs font-mono text-zinc-500">
                            <span className="text-purple-400">என்றால்</span> x &gt; 5    <span className="text-red-500"># Missing colon!</span>
                        </code>
                    </div>
                </div>
            </ContentSection>
        </>
    );
};

export default Reference;
