import React, { useState, useEffect } from 'react';
import Sidebar from '../components/Sidebar';
import GettingStarted from '../components/docs/GettingStarted';
import Basics from '../components/docs/Basics';
import ControlFlow from '../components/docs/ControlFlow';
import FunctionsDoc from '../components/docs/FunctionsDoc';
import DataStructures from '../components/docs/DataStructures';
import Reference from '../components/docs/Reference';
import AdvancedFeatures from '../components/docs/AdvancedFeatures';
import SkeletonDoc from '../components/SkeletonDoc';
import Footer from '../components/Footer';

const Docs = () => {
    const [isLoading, setIsLoading] = useState(true);

    useEffect(() => {
        // Simulate data loading
        const timer = setTimeout(() => {
            setIsLoading(false);
        }, 800);
        return () => clearTimeout(timer);
    }, []);

    const Divider = () => <div className="my-12 h-px bg-zinc-800 w-full" />;

    if (isLoading) {
        return <SkeletonDoc />;
    }

    return (
        <div className="min-h-screen bg-zinc-950 text-zinc-50 font-sans selection:bg-blue-500/30">
            <Sidebar />

            <main className="lg:pl-64 transition-all duration-300 min-h-screen">
                <div className="max-w-4xl mx-auto px-6 py-12 lg:py-16">

                    <GettingStarted />
                    <Divider />

                    <Basics />
                    <Divider />

                    <ControlFlow />
                    <Divider />

                    <FunctionsDoc />
                    <Divider />

                    <DataStructures />
                    <Divider />

                    <Reference />
                    <Divider />

                    <AdvancedFeatures />

                    <Footer />
                </div>
            </main>
        </div>
    );
};

export default Docs;
