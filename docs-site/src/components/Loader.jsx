import React from 'react';
import { motion } from 'framer-motion';

const Loader = () => {
    return (
        <div className="fixed inset-0 z-50 flex items-center justify-center bg-zinc-950">
            {/* Background Glow */}
            <div className="absolute inset-0 overflow-hidden pointer-events-none">
                <div className="absolute top-1/2 left-1/2 -translate-x-1/2 -translate-y-1/2 w-[500px] h-[500px] bg-blue-500/20 rounded-full blur-[100px] animate-pulse" />
                <div className="absolute top-1/2 left-1/2 -translate-x-1/2 -translate-y-1/2 w-[300px] h-[300px] bg-purple-500/20 rounded-full blur-[80px] animate-pulse delay-75" />
            </div>

            <div className="relative flex flex-col items-center">
                {/* Animated Bars */}
                <div className="flex gap-1.5 h-16 items-center mb-4">
                    {[...Array(5)].map((_, i) => (
                        <motion.div
                            key={i}
                            className="w-2 bg-gradient-to-t from-blue-500 to-purple-500 rounded-full"
                            animate={{
                                height: [20, 40, 20],
                                opacity: [0.5, 1, 0.5],
                            }}
                            transition={{
                                duration: 1,
                                repeat: Infinity,
                                delay: i * 0.1,
                                ease: "easeInOut",
                            }}
                        />
                    ))}
                </div>

                <motion.p
                    initial={{ opacity: 0 }}
                    animate={{ opacity: 1 }}
                    transition={{ duration: 0.5 }}
                    className="text-zinc-500 font-mono text-sm tracking-widest uppercase"
                >
                    Loading Agam Docs...
                </motion.p>
            </div>
        </div>
    );
};

export default Loader;
