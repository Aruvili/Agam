import React from 'react';
import { motion } from 'framer-motion';
import { ChevronDown } from 'lucide-react';

const Hero = ({ onGetStarted, onGitHub }) => {
    return (
        <div className="mb-20 pb-16 relative flex flex-col items-center text-center z-10">
            <motion.div
                initial={{ opacity: 0, scale: 0.9 }}
                animate={{ opacity: 1, scale: 1 }}
                transition={{ duration: 0.5 }}
                className="mb-6 inline-block"
            >
                <span className="px-3 py-1 bg-zinc-900 border border-zinc-700 rounded-full text-xs font-medium text-zinc-300">
                    v0.1.1 stable is live
                </span>
            </motion.div>

            <motion.h1
                initial={{ opacity: 0, y: 20 }}
                animate={{ opacity: 1, y: 0 }}
                transition={{ duration: 0.5, delay: 0.1 }}
                className="text-5xl md:text-7xl font-extrabold tracking-tight mb-8 bg-gradient-to-r from-blue-300 via-blue-100 to-purple-300 bg-clip-text text-transparent"
            >
                அகம் (Agam) Documentation
            </motion.h1>

            <motion.p
                initial={{ opacity: 0, y: 20 }}
                animate={{ opacity: 1, y: 0 }}
                transition={{ duration: 0.5, delay: 0.2 }}
                className="text-xl text-zinc-400 leading-relaxed max-w-2xl mb-10 mx-auto"
            >
                அகம் (Agam) is a modern, statically typed programming language built with Rust. Write code in Tamil with English compatibility. Fast, Safe, and Expressive.
            </motion.p>

            <motion.div
                initial={{ opacity: 0, y: 20 }}
                animate={{ opacity: 1, y: 0 }}
                transition={{ duration: 0.5, delay: 0.3 }}
                className="flex flex-wrap gap-4 justify-center mb-16"
            >
                <button
                    onClick={onGetStarted}
                    className="group relative px-8 py-3 bg-blue-600 hover:bg-blue-500 text-white rounded-full font-semibold transition-all hover:scale-105 active:scale-95 shadow-[0_0_20px_-5px_rgba(59,130,246,0.5)] overflow-hidden cursor-pointer"
                >
                    <span className="relative z-10">Get Started</span>
                    <div className="absolute inset-0 bg-gradient-to-r from-blue-400 to-blue-600 opacity-0 group-hover:opacity-100 transition-opacity duration-300" />
                </button>
                <button
                    onClick={onGitHub}
                    className="px-8 py-3 bg-zinc-900 hover:bg-zinc-800 border border-zinc-700 text-zinc-300 rounded-full font-semibold transition-all hover:scale-105 active:scale-95 hover:border-zinc-500 cursor-pointer"
                >
                    View on GitHub
                </button>
            </motion.div>

            {/* Scroll Indicator */}
            <motion.div
                initial={{ opacity: 0 }}
                animate={{ opacity: 1 }}
                transition={{ duration: 0.5, delay: 0.8 }}
                className="flex flex-col items-center gap-3 mt-8"
            >
                <p className="text-xs text-zinc-600 uppercase tracking-widest font-medium">
                    Meet the Team
                </p>
                <motion.div
                    animate={{
                        y: [0, 8, 0],
                    }}
                    transition={{
                        duration: 1.5,
                        repeat: Infinity,
                        ease: "easeInOut"
                    }}
                    className="flex flex-col items-center"
                >
                    <ChevronDown className="text-zinc-600" size={20} />
                </motion.div>
            </motion.div>
        </div>
    );
};

export default Hero;
