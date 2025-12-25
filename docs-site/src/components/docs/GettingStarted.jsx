import React from 'react';
import ContentSection from '../ContentSection';
import { Type, Code, Zap, ShieldCheck, Globe } from 'lucide-react';

const GettingStarted = () => {
    return (
        <>
            <ContentSection id="introduction" title="Introduction">
                <h3 className="text-xl font-semibold text-zinc-100 mb-4">What is agam?</h3>
                <p className="text-zinc-300 leading-relaxed mb-6">
                    <strong className="text-blue-400">அகம் (agam)</strong> is a programming language where you write code in Tamil! It's designed to make programming accessible to Tamil speakers by using Tamil keywords and syntax.
                </p>

                <h4 className="text-lg font-medium text-zinc-200 mb-3">Why agam?</h4>
                <div className="overflow-hidden rounded-lg border border-zinc-800 bg-zinc-900/50 mb-8">
                    <table className="w-full text-left text-sm text-zinc-400">
                        <thead className="bg-zinc-900 text-zinc-200">
                            <tr>
                                <th className="px-6 py-3 font-medium">Feature</th>
                                <th className="px-6 py-3 font-medium">Benefit</th>
                            </tr>
                        </thead>
                        <tbody className="divide-y divide-zinc-800">
                            <tr><td className="px-6 py-4 flex items-center gap-2"><Type className="text-pink-400" size={18} /> <strong>Tamil Keywords</strong></td><td className="px-6 py-4">Program using words you already know</td></tr>
                            <tr><td className="px-6 py-4 flex items-center gap-2"><Code className="text-green-400" size={18} /> <strong>Python-like Syntax</strong></td><td className="px-6 py-4">Easy to learn, familiar structure</td></tr>
                            <tr><td className="px-6 py-4 flex items-center gap-2"><Zap className="text-yellow-400" size={18} /> <strong>Fast</strong></td><td className="px-6 py-4">Built with Rust for speed</td></tr>
                            <tr><td className="px-6 py-4 flex items-center gap-2"><ShieldCheck className="text-blue-400" size={18} /> <strong>Safe</strong></td><td className="px-6 py-4">Memory-safe and type-checked</td></tr>
                            <tr><td className="px-6 py-4 flex items-center gap-2"><Globe className="text-purple-400" size={18} /> <strong>Bilingual</strong></td><td className="px-6 py-4">Supports English keywords too</td></tr>
                        </tbody>
                    </table>
                </div>

                <h3 className="text-xl font-semibold text-zinc-100 mb-4">Your First Look at agam</h3>
                <p className="text-zinc-300 mb-4">Here's what agam code looks like:</p>

                <div className="bg-zinc-900 p-4 rounded-lg border border-zinc-800 overflow-x-auto mb-8">
                    <code className="text-sm font-mono text-zinc-300">
                        <span className="text-zinc-500"># This is a comment (குறிப்பு)</span><br /><br />
                        <span className="text-purple-400">மாறி</span> <span className="text-yellow-100">பெயர்</span> <span className="text-zinc-400">=</span> <span className="text-green-400">"Tamil"</span><br />
                        <span className="text-blue-400">அச்சிடு</span>(<span className="text-green-400">"Hello, "</span> + பெயர் + <span className="text-green-400">"!"</span>)
                    </code>
                </div>

                <h3 className="text-xl font-semibold text-zinc-100 mb-4">Key Concepts</h3>
                <div className="grid grid-cols-1 md:grid-cols-2 gap-6 mb-8">
                    <div className="p-5 bg-zinc-900/30 border border-zinc-800 rounded-lg">
                        <h4 className="text-white font-medium mb-3">Keywords (முக்கிய சொற்கள்)</h4>
                        <ul className="space-y-2 text-sm text-zinc-400">
                            <li className="flex justify-between border-b border-zinc-800 pb-1"><span>மாறி (let)</span> <span>Create a variable</span></li>
                            <li className="flex justify-between border-b border-zinc-800 pb-1"><span>செயல் (function)</span> <span>Create a function</span></li>
                            <li className="flex justify-between border-b border-zinc-800 pb-1"><span>அச்சிடு (print)</span> <span>Display output</span></li>
                            <li className="flex justify-between border-b border-zinc-800 pb-1"><span>என்றால் (if)</span> <span>Make a decision</span></li>
                            <li className="flex justify-between pb-1"><span>வரை (while)</span> <span>Repeat code</span></li>
                        </ul>
                    </div>
                    <div className="p-5 bg-zinc-900/30 border border-zinc-800 rounded-lg">
                        <h4 className="text-white font-medium mb-3">Indentation (உள்தள்ளுதல்)</h4>
                        <p className="text-sm text-zinc-400 mb-2">Like Python, agam uses <strong>spaces</strong> to group code.</p>
                        <div className="bg-zinc-950 p-3 rounded border border-zinc-800 font-mono text-xs">
                            <span className="text-purple-400">என்றால்</span> 10 &gt; 5:<br />
                            &nbsp;&nbsp;&nbsp;&nbsp;<span className="text-blue-400">அச்சிடு</span>(<span className="text-green-400">"Ten is bigger!"</span>)<br />
                            <span className="text-blue-400">அச்சிடு</span>(<span className="text-green-400">"Outside"</span>)
                        </div>
                    </div>
                </div>
            </ContentSection>

            <ContentSection id="installation" title="Installation">
                <p className="text-zinc-300 mb-6">
                    Before installing agam, ensure you have <strong>Rust</strong> (v1.70+) and <strong>Cargo</strong> installed.
                </p>

                <h3 className="text-xl font-semibold text-zinc-100 mb-4">Installing agam</h3>

                <div className="mb-6">
                    <h4 className="text-lg font-medium text-zinc-200 mb-2">Option 1: Build from Source (Recommended)</h4>
                    <div className="bg-zinc-900 p-4 rounded-lg border border-zinc-800 overflow-x-auto text-sm">
                        <code className="block text-zinc-400 font-mono mb-2"># Step 1: Navigate to the Language directory</code>
                        <code className="block text-white font-mono mb-4">cd d:\agam\Language</code>

                        <code className="block text-zinc-400 font-mono mb-2"># Step 2: Build the release version</code>
                        <code className="block text-white font-mono mb-4">cargo build --release</code>

                        <code className="block text-zinc-400 font-mono"># Binary location: target/release/agam</code>
                    </div>
                </div>

                <div className="mb-6">
                    <h4 className="text-lg font-medium text-zinc-200 mb-2">Option 2: Add to PATH</h4>
                    <div className="grid md:grid-cols-2 gap-4">
                        <div className="bg-zinc-900/50 p-4 rounded-lg border border-zinc-800">
                            <span className="block text-xs uppercase tracking-wider text-zinc-500 mb-2 font-semibold">Windows (PowerShell)</span>
                            <code className="text-xs text-blue-300 break-all">
                                $env:Path += ";D:\agam\Language\target\release"<br />
                                [Environment]::SetEnvironmentVariable...
                            </code>
                        </div>
                        <div className="bg-zinc-900/50 p-4 rounded-lg border border-zinc-800">
                            <span className="block text-xs uppercase tracking-wider text-zinc-500 mb-2 font-semibold">Linux / macOS</span>
                            <code className="text-xs text-blue-300">
                                export PATH="$PATH:/path/to/agam..."
                            </code>
                        </div>
                    </div>
                </div>

                <h3 className="text-xl font-semibold text-zinc-100 mb-4">Testing Installation</h3>
                <p className="text-zinc-300 mb-3">Create a file called <code>test.agam</code>:</p>
                <div className="bg-zinc-900 p-3 rounded-lg border border-zinc-800 mb-4">
                    <code className="text-sm font-mono text-zinc-300">
                        <span className="text-blue-400">அச்சிடு</span>(<span className="text-green-400">"agam நிறுவல் வெற்றி!"</span>)<br />
                        <span className="text-blue-400">அச்சிடு</span>(<span className="text-green-400">"Installation successful!"</span>)
                    </code>
                </div>
            </ContentSection>

            <ContentSection id="hello-world" title="Hello World">
                <p className="text-zinc-300 mb-6">
                    Let's write your first program in Tamil! (வணக்கம் உலகம்)
                </p>
                <div className="relative group mb-8">
                    <div className="absolute -inset-0.5 bg-gradient-to-r from-pink-600 to-purple-600 rounded-lg blur opacity-20 group-hover:opacity-40 transition duration-1000"></div>
                    <div className="relative bg-zinc-900 p-6 rounded-lg border border-zinc-800">
                        <div className="flex justify-between items-center mb-4 border-b border-zinc-800 pb-2">
                            <span className="text-xs text-zinc-500 font-mono">hello.agam</span>
                            <div className="flex gap-1.5">
                                <div className="w-2.5 h-2.5 rounded-full bg-red-500/20 border border-red-500/50"></div>
                                <div className="w-2.5 h-2.5 rounded-full bg-yellow-500/20 border border-yellow-500/50"></div>
                                <div className="w-2.5 h-2.5 rounded-full bg-green-500/20 border border-green-500/50"></div>
                            </div>
                        </div>
                        <code className="text-base font-mono text-zinc-300">
                            <span className="text-blue-400">அச்சிடு</span>(<span className="text-green-400">"வணக்கம் உலகம்!"</span>)
                        </code>
                    </div>
                </div>

                <h3 className="text-xl font-semibold text-zinc-100 mb-4">Running the Code</h3>
                <div className="bg-black/30 p-4 rounded-md border border-zinc-800 font-mono text-sm text-zinc-400 mb-8">
                    $ cargo run --release -- hello.agam<br />
                    <span className="text-white mt-2 block">வணக்கம் உலகம்!</span>
                </div>

                <h3 className="text-xl font-semibold text-zinc-100 mb-4">More Examples</h3>
                <div className="grid gap-4">
                    <div className="p-4 rounded-lg bg-zinc-900/50 border border-zinc-800 font-mono text-sm">
                        <div className="text-zinc-500 mb-2"># Print Numbers</div>
                        <span className="text-blue-400">அச்சிடு</span>(<span className="text-orange-300">42</span>)<br />
                        <span className="text-blue-400">அச்சிடு</span>(<span className="text-orange-300">3.14159</span>)
                    </div>
                    <div className="p-4 rounded-lg bg-zinc-900/50 border border-zinc-800 font-mono text-sm">
                        <div className="text-zinc-500 mb-2"># Print Calculations</div>
                        <span className="text-blue-400">அச்சிடு</span>(<span className="text-orange-300">10</span> + <span className="text-orange-300">5</span>) <span className="text-zinc-600">// Output: 15</span>
                    </div>
                </div>
            </ContentSection>
        </>
    );
};

export default GettingStarted;
