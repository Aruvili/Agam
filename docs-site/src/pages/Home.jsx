import React, { useState, useEffect } from 'react';
import { useNavigate } from 'react-router-dom';
import Hero from '../components/Hero';
import Loader from '../components/Loader';
import Team from '../components/Team';
import Footer from '../components/Footer';

const Home = () => {
    const navigate = useNavigate();
    const [isLoading, setIsLoading] = useState(true);

    useEffect(() => {
        // Simulate page load for Home
        const timer = setTimeout(() => {
            setIsLoading(false);
        }, 1000);
        return () => clearTimeout(timer);
    }, []);

    const handleGetStarted = () => {
        setIsLoading(true);

        setTimeout(() => {
            navigate('/docs');
        }, 1500);
    };

    const handleGitHub = () => {
        window.open('https://github.com/Aruvili/Agam', '_blank');
    };

    if (isLoading) {
        return <Loader />;
    }

    return (
        <div className="min-h-screen text-zinc-50 font-sans selection:bg-blue-500/30 relative overflow-x-hidden">
            {/* 1. Solid Dark Background */}
            <div className="fixed inset-0 bg-zinc-950 -z-20" />

            {/* 2. Lightweight CSS Gradient Background */}
            <div className="fixed inset-0 -z-10 pointer-events-none overflow-hidden">
                <div className="absolute inset-0 bg-gradient-to-b from-blue-950/20 via-purple-950/10 to-zinc-950/0"></div>
                <div className="absolute top-0 left-1/2 -translate-x-1/2 w-[800px] h-[800px] bg-gradient-radial from-blue-500/10 via-purple-500/5 to-transparent rounded-full blur-3xl animate-pulse-slow"></div>
                <div className="absolute top-1/4 right-0 w-[600px] h-[600px] bg-gradient-radial from-purple-500/10 via-pink-500/5 to-transparent rounded-full blur-3xl animate-pulse-slow-delayed"></div>
            </div>

            <div className="relative w-full">
                <div className="max-w-4xl mx-auto px-6 pt-20 pb-12 lg:pt-32 lg:pb-16 w-full">
                    <Hero onGetStarted={handleGetStarted} onGitHub={handleGitHub} />
                </div>

                <div className="w-full">
                    <Team />
                </div>

                <div className="max-w-4xl mx-auto px-6 w-full">
                    <Footer />
                </div>
            </div>
        </div>
    );
};

export default Home;
