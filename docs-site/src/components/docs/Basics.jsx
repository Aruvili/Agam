import React from 'react';
import ContentSection from '../ContentSection';

const Basics = () => {
    return (
        <>
            <ContentSection id="variables" title="Variables">
                <h3 className="text-xl font-semibold text-zinc-100 mb-4">What is a Variable?</h3>
                <p className="text-zinc-300 mb-4">
                    A <strong>variable</strong> is like a labeled box that stores data. You give it a name, and it remembers a value for you.
                </p>
                <div className="bg-zinc-900 p-4 rounded-lg border border-zinc-800 font-mono text-sm mb-6">
                    <span className="text-purple-400">மாறி</span> <span className="text-yellow-100">பெயர்</span> <span className="text-zinc-400">=</span> <span className="text-green-400">"agam"</span><br />
                    <span className="text-zinc-500 text-xs">#     ↑      ↑</span><br />
                    <span className="text-zinc-500 text-xs">#   name   value</span>
                </div>

                <h3 className="text-xl font-semibold text-zinc-100 mb-4">Creating Variables</h3>
                <p className="text-zinc-300 mb-4">
                    Use <code className="text-purple-400">மாறி</code> (meaning "variable" or "let") to create a variable:
                </p>
                <div className="bg-zinc-900 p-4 rounded-lg border border-zinc-800 font-mono text-sm mb-8">
                    <span className="text-purple-400">மாறி</span> <span className="text-yellow-100">பெயர்</span> <span className="text-zinc-400">=</span> <span className="text-green-400">"Tamil"</span><br />
                    <span className="text-purple-400">மாறி</span> <span className="text-yellow-100">வயது</span> <span className="text-zinc-400">=</span> <span className="text-orange-300">25</span><br />
                    <span className="text-purple-400">மாறி</span> <span className="text-yellow-100">விலை</span> <span className="text-zinc-400">=</span> <span className="text-orange-300">99.50</span><br /><br />
                    <span className="text-blue-400">அச்சிடு</span>(பெயர்)   <span className="text-zinc-500"># Output: Tamil</span>
                </div>

                <h3 className="text-xl font-semibold text-zinc-100 mb-4">Constants (மாறாத)</h3>
                <p className="text-zinc-300 mb-4">
                    Use <code className="text-purple-400">மாறாத</code> (meaning "constant" or "unchanging") for values that should never change:
                </p>
                <div className="bg-zinc-900 p-4 rounded-lg border border-zinc-800 font-mono text-sm">
                    <span className="text-purple-400">மாறாத</span> <span className="text-yellow-100">PI</span> <span className="text-zinc-400">=</span> <span className="text-orange-300">3.14159</span><br />
                    <span className="text-blue-400">அச்சிடு</span>(PI)         <span className="text-zinc-500"># Output: 3.14159</span>
                </div>
            </ContentSection>

            <ContentSection id="data-types" title="Data Types">
                <p className="text-zinc-300 mb-6">
                    Data types tell the computer what kind of information you're working with. agam has several built-in types:
                </p>

                <div className="overflow-hidden rounded-lg border border-zinc-800 bg-zinc-900/50 mb-8">
                    <table className="w-full text-left text-sm text-zinc-400">
                        <thead className="bg-zinc-900 text-zinc-200">
                            <tr>
                                <th className="px-4 py-2 font-medium">Type</th>
                                <th className="px-4 py-2 font-medium">Tamil</th>
                                <th className="px-4 py-2 font-medium">Example</th>
                            </tr>
                        </thead>
                        <tbody className="divide-y divide-zinc-800">
                            <tr><td className="px-4 py-2">Number</td><td className="px-4 py-2 font-mono text-purple-300">எண்</td><td className="px-4 py-2 font-mono text-orange-300">42, 3.14</td></tr>
                            <tr><td className="px-4 py-2">String</td><td className="px-4 py-2 font-mono text-purple-300">சரம்</td><td className="px-4 py-2 font-mono text-green-300">"Hello"</td></tr>
                            <tr><td className="px-4 py-2">Boolean</td><td className="px-4 py-2 font-mono text-purple-300">உண்மைபொய்</td><td className="px-4 py-2 font-mono text-blue-300">உண்மை, பொய்</td></tr>
                            <tr><td className="px-4 py-2">List</td><td className="px-4 py-2 font-mono text-purple-300">பட்டியல்</td><td className="px-4 py-2 font-mono">[1, 2, 3]</td></tr>
                            <tr><td className="px-4 py-2">Null</td><td className="px-4 py-2 font-mono text-purple-300">இல்லா</td><td className="px-4 py-2 font-mono text-red-300">இல்லா</td></tr>
                        </tbody>
                    </table>
                </div>

                <div className="grid md:grid-cols-2 gap-4">
                    <div className="bg-zinc-900/50 p-4 rounded-lg border border-zinc-800">
                        <h4 className="text-white font-medium mb-2">Numbers (எண்)</h4>
                        <code className="text-xs font-mono text-zinc-400 block">
                            <span className="text-purple-400">மாறி</span> age = <span className="text-orange-300">25</span><br />
                            <span className="text-purple-400">மாறி</span> pi = <span className="text-orange-300">3.14159</span>
                        </code>
                    </div>
                    <div className="bg-zinc-900/50 p-4 rounded-lg border border-zinc-800">
                        <h4 className="text-white font-medium mb-2">Strings (சரம்)</h4>
                        <code className="text-xs font-mono text-zinc-400 block">
                            <span className="text-purple-400">மாறி</span> greeting = <span className="text-green-300">"வணக்கம்!"</span>
                        </code>
                    </div>
                    <div className="bg-zinc-900/50 p-4 rounded-lg border border-zinc-800">
                        <h4 className="text-white font-medium mb-2">Booleans (உண்மைபொய்)</h4>
                        <code className="text-xs font-mono text-zinc-400 block">
                            <span className="text-purple-400">மாறி</span> is_student = <span className="text-blue-300">உண்மை</span><br />
                            <span className="text-purple-400">மாறி</span> is_old = <span className="text-blue-300">பொய்</span>
                        </code>
                    </div>
                    <div className="bg-zinc-900/50 p-4 rounded-lg border border-zinc-800">
                        <h4 className="text-white font-medium mb-2">Null (இல்லா)</h4>
                        <code className="text-xs font-mono text-zinc-400 block">
                            <span className="text-purple-400">மாறி</span> data = <span className="text-red-300">இல்லா</span>
                        </code>
                    </div>
                </div>
            </ContentSection>

            <ContentSection id="operators" title="Operators">
                <p className="text-zinc-300 mb-6">
                    Operators are symbols that perform operations on values.
                </p>

                <h3 className="text-xl font-semibold text-zinc-100 mb-4">Arithmetic Operators (கணித செயற்குறிகள்)</h3>
                <div className="overflow-hidden rounded-lg border border-zinc-800 bg-zinc-900/50 mb-8">
                    <table className="w-full text-left text-sm text-zinc-400">
                        <thead className="bg-zinc-900 text-zinc-200">
                            <tr>
                                <th className="px-4 py-2 font-medium">Operator</th>
                                <th className="px-4 py-2 font-medium">Example</th>
                                <th className="px-4 py-2 font-medium">Result</th>
                            </tr>
                        </thead>
                        <tbody className="divide-y divide-zinc-800">
                            <tr><td className="px-4 py-2 font-mono">+ (Add)</td><td className="px-4 py-2 font-mono">10 + 5</td><td className="px-4 py-2 font-mono">15</td></tr>
                            <tr><td className="px-4 py-2 font-mono">- (Sub)</td><td className="px-4 py-2 font-mono">10 - 5</td><td className="px-4 py-2 font-mono">5</td></tr>
                            <tr><td className="px-4 py-2 font-mono">* (Mul)</td><td className="px-4 py-2 font-mono">10 * 5</td><td className="px-4 py-2 font-mono">50</td></tr>
                            <tr><td className="px-4 py-2 font-mono">/ (Div)</td><td className="px-4 py-2 font-mono">10 / 4</td><td className="px-4 py-2 font-mono">2.5</td></tr>
                            <tr><td className="px-4 py-2 font-mono">% (Mod)</td><td className="px-4 py-2 font-mono">10 % 3</td><td className="px-4 py-2 font-mono">1</td></tr>
                        </tbody>
                    </table>
                </div>

                <h3 className="text-xl font-semibold text-zinc-100 mb-4">Logical Operators (தருக்க செயற்குறிகள்)</h3>
                <div className="grid gap-4">
                    <div className="bg-zinc-900/50 p-4 rounded-lg border border-zinc-800">
                        <h4 className="text-white font-medium mb-1">மற்றும் (AND)</h4>
                        <p className="text-xs text-zinc-400 mb-2">Both conditions must be true</p>
                        <code className="text-xs font-mono text-zinc-300">
                            <span className="text-purple-400">என்றால்</span> age &gt;= 18 <span className="text-blue-400">மற்றும்</span> has_license:
                        </code>
                    </div>
                    <div className="bg-zinc-900/50 p-4 rounded-lg border border-zinc-800">
                        <h4 className="text-white font-medium mb-1">அல்லது (OR)</h4>
                        <p className="text-xs text-zinc-400 mb-2">At least one condition must be true</p>
                        <code className="text-xs font-mono text-zinc-300">
                            <span className="text-purple-400">என்றால்</span> is_weekend <span className="text-blue-400">அல்லது</span> is_holiday:
                        </code>
                    </div>
                    <div className="bg-zinc-900/50 p-4 rounded-lg border border-zinc-800">
                        <h4 className="text-white font-medium mb-1">இல்ல (NOT)</h4>
                        <p className="text-xs text-zinc-400 mb-2">Reverses the boolean value</p>
                        <code className="text-xs font-mono text-zinc-300">
                            <span className="text-purple-400">என்றால்</span> <span className="text-blue-400">இல்ல</span> is_raining:
                        </code>
                    </div>
                </div>
            </ContentSection>
        </>
    );
};

export default Basics;
