import React from 'react';
import ContentSection from '../ContentSection';

const DataStructures = () => {
    return (
        <>
            <ContentSection id="lists" title="Lists">
                <h3 className="text-xl font-semibold text-zinc-100 mb-4">What is a List?</h3>
                <p className="text-zinc-300 mb-4">
                    A list is an ordered collection of items. Items can be of any type.
                </p>
                <div className="bg-zinc-900 p-4 rounded-lg border border-zinc-800 font-mono text-sm mb-6">
                    <span className="text-purple-400">மாறி</span> numbers = [<span className="text-orange-300">1</span>, <span className="text-orange-300">2</span>, <span className="text-orange-300">3</span>, <span className="text-orange-300">4</span>, <span className="text-orange-300">5</span>]<br />
                    <span className="text-purple-400">மாறி</span> mixed = [<span className="text-orange-300">1</span>, <span className="text-green-300">"hello"</span>, <span className="text-blue-300">உண்மை</span>]
                </div>

                <h3 className="text-xl font-semibold text-zinc-100 mb-4">Common Operations</h3>
                <div className="grid gap-4">
                    <div className="bg-zinc-900/50 p-3 rounded border border-zinc-800 flex justify-between items-center text-sm">
                        <span className="text-zinc-400">Access Element</span>
                        <code className="font-mono text-zinc-300">list[0]</code>
                    </div>
                    <div className="bg-zinc-900/50 p-3 rounded border border-zinc-800 flex justify-between items-center text-sm">
                        <span className="text-zinc-400">Add Item</span>
                        <code className="font-mono text-zinc-300"><span className="text-blue-400">சேர்</span>(list, item)</code>
                    </div>
                    <div className="bg-zinc-900/50 p-3 rounded border border-zinc-800 flex justify-between items-center text-sm">
                        <span className="text-zinc-400">Remove Item</span>
                        <code className="font-mono text-zinc-300"><span className="text-blue-400">நீக்கு</span>(list)</code>
                    </div>
                </div>
            </ContentSection>

            <ContentSection id="dictionaries" title="Dictionaries">
                <h3 className="text-xl font-semibold text-zinc-100 mb-4">What is a Dictionary?</h3>
                <p className="text-zinc-300 mb-4">
                    A dictionary stores data as <strong>key-value pairs</strong>.
                </p>
                <div className="bg-zinc-900 p-4 rounded-lg border border-zinc-800 font-mono text-sm mb-6">
                    <span className="text-purple-400">மாறி</span> person = {'{'}<br />
                    &nbsp;&nbsp;&nbsp;&nbsp;<span className="text-green-300">"name"</span>: <span className="text-green-300">"Tamil"</span>,<br />
                    &nbsp;&nbsp;&nbsp;&nbsp;<span className="text-green-300">"age"</span>: <span className="text-orange-300">25</span>,<br />
                    &nbsp;&nbsp;&nbsp;&nbsp;<span className="text-green-300">"city"</span>: <span className="text-green-300">"Chennai"</span><br />
                    {'}'}
                </div>

                <h3 className="text-xl font-semibold text-zinc-100 mb-4">Accessing Values</h3>
                <div className="bg-zinc-900 p-4 rounded-lg border border-zinc-800 font-mono text-sm">
                    <span className="text-blue-400">அச்சிடு</span>(person[<span className="text-green-300">"name"</span>])   <span className="text-zinc-500"># Output: Tamil</span>
                </div>
            </ContentSection>
        </>
    );
};

export default DataStructures;
