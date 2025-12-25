import React from 'react';
import ContentSection from '../ContentSection';

const ControlFlow = () => {
    return (
        <>
            <ContentSection id="conditionals" title="Conditionals">
                <p className="text-zinc-300 mb-4">
                    Conditionals let your program make decisions based on conditions.
                </p>
                <div className="bg-zinc-900 p-4 rounded-lg border border-zinc-800 font-mono text-sm mb-6">
                    <span className="text-purple-400">மாறி</span> age = <span className="text-orange-300">20</span><br /><br />
                    <span className="text-purple-400">என்றால்</span> age &gt;= <span className="text-orange-300">18</span>:<br />
                    &nbsp;&nbsp;&nbsp;&nbsp;<span className="text-blue-400">அச்சிடு</span>(<span className="text-green-300">"You are an adult!"</span>)
                </div>

                <h3 className="text-xl font-semibold text-zinc-100 mb-4">Keyword Reference</h3>
                <div className="overflow-hidden rounded-lg border border-zinc-800 bg-zinc-900/50 mb-8">
                    <table className="w-full text-left text-sm text-zinc-400">
                        <thead className="bg-zinc-900 text-zinc-200">
                            <tr>
                                <th className="px-4 py-2 font-medium">Tamil</th>
                                <th className="px-4 py-2 font-medium">English</th>
                                <th className="px-4 py-2 font-medium">Usage</th>
                            </tr>
                        </thead>
                        <tbody className="divide-y divide-zinc-800">
                            <tr><td className="px-4 py-2 font-mono text-purple-300">என்றால்</td><td className="px-4 py-2">if</td><td className="px-4 py-2">First condition</td></tr>
                            <tr><td className="px-4 py-2 font-mono text-purple-300">இல்லையென்றால்</td><td className="px-4 py-2">elif</td><td className="px-4 py-2">Additional conditions</td></tr>
                            <tr><td className="px-4 py-2 font-mono text-purple-300">இல்லை</td><td className="px-4 py-2">else</td><td className="px-4 py-2">When no condition matches</td></tr>
                        </tbody>
                    </table>
                </div>
                <p className="text-zinc-400 text-sm italic">
                    <strong>Important:</strong> Don't forget the <code>:</code> after the condition and indent the code inside!
                </p>
            </ContentSection>

            <ContentSection id="loops" title="Loops">
                <p className="text-zinc-300 mb-4">
                    Loops let you repeat code multiple times without writing it over and over.
                </p>

                <h3 className="text-xl font-semibold text-zinc-100 mb-4">The வரை Loop (while)</h3>
                <div className="bg-zinc-900 p-4 rounded-lg border border-zinc-800 font-mono text-sm mb-6">
                    <span className="text-purple-400">மாறி</span> count = <span className="text-orange-300">1</span><br /><br />
                    <span className="text-purple-400">வரை</span> count &lt;= <span className="text-orange-300">5</span>:<br />
                    &nbsp;&nbsp;&nbsp;&nbsp;<span className="text-blue-400">அச்சிடு</span>(count)<br />
                    &nbsp;&nbsp;&nbsp;&nbsp;count = count + <span className="text-orange-300">1</span>
                </div>
            </ContentSection>
        </>
    );
};

export default ControlFlow;
