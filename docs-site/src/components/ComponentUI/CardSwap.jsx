import React, { useState, useEffect } from "react";
import { motion } from "framer-motion";

const CardSwap = ({ items, renderItem }) => {
    const [cards, setCards] = useState(items);

    const moveToEnd = (from) => {
        setCards((currentCards) => {
            const newCards = [...currentCards];
            const movedItem = newCards.splice(from, 1)[0];
            newCards.push(movedItem);
            return newCards;
        });
    };

    const [intervalId, setIntervalId] = useState(null);

    useEffect(() => {
        const id = setInterval(() => {
            moveToEnd(0);
        }, 5000);
        setIntervalId(id);

        return () => clearInterval(id);
    }, []);

    const handlePause = () => {
        if (intervalId) {
            clearInterval(intervalId);
            setIntervalId(null);
        }
    };

    const handleResume = () => {
        if (!intervalId) {
            const id = setInterval(() => {
                moveToEnd(0);
            }, 5000);
            setIntervalId(id);
        }
    };

    return (
        <div
            className="relative w-full max-w-sm mx-auto h-[400px] flex items-center justify-center cursor-pointer perspective-1000"
            onMouseEnter={handlePause}
            onMouseLeave={handleResume}
            onClick={() => moveToEnd(0)}
        >
            {cards.map((item, index) => {
                // Only render the top 3 items for performance
                if (index > 2) return null;

                return (
                    <motion.div
                        key={item.id}
                        layoutId={item.id}
                        initial={false}
                        animate={{
                            scale: 1 - index * 0.05,
                            y: index * 15, // Offset down for stacked look
                            z: -index * 50, // Move back in 3D space
                            zIndex: cards.length - index,
                            opacity: index === 0 ? 1 : index === 1 ? 0.9 : 0.5,
                        }}
                        transition={{
                            type: "spring",
                            stiffness: 300,
                            damping: 30,
                        }}
                        style={{
                            transformStyle: "preserve-3d",
                            width: "100%",
                            height: "100%",
                            position: "absolute",
                            top: 0,
                            transformOrigin: "top center"
                        }}
                        className="w-full h-full shadow-2xl rounded-2xl"
                    >
                        {renderItem(item)}
                    </motion.div>
                );
            })}
        </div>
    );
};

export default CardSwap;
