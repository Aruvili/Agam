import React from 'react';
import ContentSection from '../ContentSection';

const AdvancedFeatures = () => {
    return (
        <>
            <ContentSection id="structs" title="Structs">
                <p className="text-zinc-300 mb-6">
                    Structs allow you to create custom data types by grouping related values together. In agam, you use <code className="text-purple-400">கட்டமைப்பு</code> (struct).
                </p>
                <div className="bg-zinc-900 p-4 rounded-lg border border-zinc-800 font-mono text-sm mb-6">
                    <span className="text-purple-400">கட்டமைப்பு</span> Person:<br />
                    &nbsp;&nbsp;&nbsp;&nbsp;name<br />
                    &nbsp;&nbsp;&nbsp;&nbsp;age<br />
                    &nbsp;&nbsp;&nbsp;&nbsp;city
                </div>

                <h3 className="text-xl font-semibold text-zinc-100 mb-4">Creating & Accessing</h3>
                <div className="bg-zinc-900 p-4 rounded-lg border border-zinc-800 font-mono text-sm">
                    <span className="text-zinc-500"># Create a person</span><br />
                    <span className="text-purple-400">மாறி</span> person1 = Person(<span className="text-green-300">"ராஜா"</span>, <span className="text-orange-300">25</span>, <span className="text-green-300">"சென்னை"</span>)<br /><br />
                    <span className="text-zinc-500"># Access fields</span><br />
                    <span className="text-blue-400">அச்சிடு</span>(person1.name)    <span className="text-zinc-500"># Output: ராஜா</span>
                </div>
            </ContentSection>

            <ContentSection id="enums" title="Enums">
                <p className="text-zinc-300 mb-6">
                    Enums (enumerations) allow you to define a type with a fixed set of possible values. Use <code className="text-purple-400">விருப்பம்</code> (enum).
                </p>
                <div className="bg-zinc-900 p-4 rounded-lg border border-zinc-800 font-mono text-sm mb-6">
                    <span className="text-purple-400">விருப்பம்</span> Color:<br />
                    &nbsp;&nbsp;&nbsp;&nbsp;Red<br />
                    &nbsp;&nbsp;&nbsp;&nbsp;Green<br />
                    &nbsp;&nbsp;&nbsp;&nbsp;Blue
                </div>

                <h3 className="text-xl font-semibold text-zinc-100 mb-4">Using Enums</h3>
                <div className="bg-zinc-900 p-4 rounded-lg border border-zinc-800 font-mono text-sm">
                    <span className="text-purple-400">மாறி</span> favorite = Color.Red<br />
                    <span className="text-blue-400">அச்சிடு</span>(favorite)    <span className="text-zinc-500"># Output: Color.Red</span>
                </div>
            </ContentSection>

            <ContentSection id="pattern-matching" title="Pattern Matching">
                <p className="text-zinc-300 mb-6">
                    Pattern matching lets you match values against patterns using <code className="text-purple-400">பொருத்து</code> (match).
                </p>
                <div className="bg-zinc-900 p-4 rounded-lg border border-zinc-800 font-mono text-sm mb-6">
                    <span className="text-purple-400">பொருத்து</span> value:<br />
                    &nbsp;&nbsp;&nbsp;&nbsp;pattern1 =&gt; result1<br />
                    &nbsp;&nbsp;&nbsp;&nbsp;pattern2 =&gt; result2<br />
                    &nbsp;&nbsp;&nbsp;&nbsp;_ =&gt; default_result
                </div>

                <h3 className="text-xl font-semibold text-zinc-100 mb-4">Example</h3>
                <div className="bg-zinc-900 p-4 rounded-lg border border-zinc-800 font-mono text-sm">
                    <span className="text-purple-400">மாறி</span> grade = <span className="text-orange-300">85</span><br /><br />
                    <span className="text-purple-400">பொருத்து</span> grade:<br />
                    &nbsp;&nbsp;&nbsp;&nbsp;<span className="text-orange-300">100</span> =&gt; <span className="text-blue-400">அச்சிடு</span>(<span className="text-green-300">"Perfect score!"</span>)<br />
                    &nbsp;&nbsp;&nbsp;&nbsp;<span className="text-orange-300">90</span> =&gt; <span className="text-blue-400">அச்சிடு</span>(<span className="text-green-300">"Excellent! A+"</span>)<br />
                    &nbsp;&nbsp;&nbsp;&nbsp;_ =&gt; <span className="text-blue-400">அச்சிடு</span>(<span className="text-green-300">"Keep working!"</span>)
                </div>
            </ContentSection>

            <ContentSection id="error-handling" title="Error Handling">
                <p className="text-zinc-300 mb-6">
                    Error handling allows your program to gracefully handle unexpected situations using <code className="text-purple-400">முயற்சி</code> (try), <code className="text-purple-400">பிடி</code> (catch), and <code className="text-purple-400">வீசு</code> (throw).
                </p>
                <div className="bg-zinc-900 p-4 rounded-lg border border-zinc-800 font-mono text-sm mb-6">
                    <span className="text-purple-400">முயற்சி</span>:<br />
                    &nbsp;&nbsp;&nbsp;&nbsp;<span className="text-purple-400">மாறி</span> result = <span className="text-orange-300">10</span> / <span className="text-orange-300">0</span><br />
                    &nbsp;&nbsp;&nbsp;&nbsp;<span className="text-blue-400">அச்சிடு</span>(result)<br />
                    <span className="text-purple-400">பிடி</span> error:<br />
                    &nbsp;&nbsp;&nbsp;&nbsp;<span className="text-blue-400">அச்சிடு</span>(<span className="text-green-300">"An error occurred!"</span>)<br />
                    &nbsp;&nbsp;&nbsp;&nbsp;<span className="text-blue-400">அச்சிடு</span>(<span className="text-green-300">"Details:"</span>, error)
                </div>

                <h3 className="text-xl font-semibold text-zinc-100 mb-4">Throwing Errors</h3>
                <div className="bg-zinc-900 p-4 rounded-lg border border-zinc-800 font-mono text-sm">
                    <span className="text-purple-400">என்றால்</span> age &lt; <span className="text-orange-300">0</span>:<br />
                    &nbsp;&nbsp;&nbsp;&nbsp;<span className="text-purple-400">வீசு</span>(<span className="text-green-300">"Age cannot be negative!"</span>)
                </div>
            </ContentSection>

            <ContentSection id="modules" title="Modules">
                <p className="text-zinc-300 mb-6">
                    Modules allow you to organize your code and reuse it. Use <code className="text-purple-400">இறக்குமதி</code> (import) and <code className="text-purple-400">இருந்து</code> (from).
                </p>

                <h3 className="text-xl font-semibold text-zinc-100 mb-4">Importing</h3>
                <div className="bg-zinc-900 p-4 rounded-lg border border-zinc-800 font-mono text-sm mb-6">
                    <span className="text-zinc-500"># Basic Import</span><br />
                    <span className="text-purple-400">இறக்குமதி</span> math_utils<br /><br />
                    <span className="text-zinc-500"># Specific Import</span><br />
                    <span className="text-purple-400">இருந்து</span> math_utils <span className="text-purple-400">இறக்குமதி</span> add, subtract
                </div>

                <h3 className="text-xl font-semibold text-zinc-100 mb-4">Using Modules</h3>
                <div className="bg-zinc-900 p-4 rounded-lg border border-zinc-800 font-mono text-sm">
                    <span className="text-purple-400">மாறி</span> result = math_utils.add(<span className="text-orange-300">5</span>, <span className="text-orange-300">3</span>)<br />
                    <span className="text-blue-400">அச்சிடு</span>(result)
                </div>
            </ContentSection>

            <ContentSection id="file-io" title="File I/O">
                <p className="text-zinc-300 mb-6">
                    Read and write files using built-in functions.
                </p>

                <h3 className="text-xl font-semibold text-zinc-100 mb-4">Reading Files (படி)</h3>
                <div className="bg-zinc-900 p-4 rounded-lg border border-zinc-800 font-mono text-sm mb-6">
                    <span className="text-purple-400">மாறி</span> content = <span className="text-blue-400">படி</span>(<span className="text-green-300">"data.txt"</span>)<br />
                    <span className="text-blue-400">அச்சிடு</span>(content)
                </div>

                <h3 className="text-xl font-semibold text-zinc-100 mb-4">Writing Files (எழுது)</h3>
                <div className="bg-zinc-900 p-4 rounded-lg border border-zinc-800 font-mono text-sm">
                    <span className="text-purple-400">மாறி</span> content = <span className="text-green-300">"வணக்கம் உலகம்!"</span><br />
                    <span className="text-blue-400">எழுது</span>(<span className="text-green-300">"output.txt"</span>, content)<br />
                    <span className="text-blue-400">அச்சிடு</span>(<span className="text-green-300">"File written!"</span>)
                </div>
            </ContentSection>
        </>
    );
};

export default AdvancedFeatures;
