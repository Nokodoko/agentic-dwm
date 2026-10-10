package main

import (
	"fmt"
	"time"
)

// PromptContext demonstrates the concept of prompt construction with context injection
type PromptContext struct {
	turnCount   int
	threshold   int
	totalTokens int
	usedTokens  int
	available   int
}

func NewPromptContext() *PromptContext {
	return &PromptContext{
		turnCount:   0,
		threshold:   3, // Default threshold for injecting context usage
		totalTokens: 240518,
		usedTokens:  21970,
		available:   218548,
	}
}

func (pc *PromptContext) IncrementTurn() {
	pc.turnCount++
}

func (pc *PromptContext) ShouldInjectContextUsage() bool {
	return pc.turnCount >= pc.threshold
}

func (pc *PromptContext) GetContextUsageString() string {
	percentUsed := float64(pc.usedTokens) / float64(pc.totalTokens) * 100
	return fmt.Sprintf("[context] session start: %d of %d tokens used (%.0f%%); %d tokens available; compaction (/cmp --clear) fires at 70%%.",
		pc.usedTokens, pc.totalTokens, percentUsed, pc.available)
}

func (pc *PromptContext) InjectContextIntoPrompt(prompt string) string {
	if !pc.ShouldInjectContextUsage() {
		return prompt // Don't inject context usage yet
	}

	// Inject context usage at the end of the prompt
	contextUsage := pc.GetContextUsageString()
	return fmt.Sprintf("%s\n\n[Context Usage Info]\n%s", prompt, contextUsage)
}

func main() {
	ctx := NewPromptContext()

	// Simulate conversation with context injection
	prompt := "User: What is the current system status?"

	fmt.Println("Initial prompt:", prompt)
	fmt.Println()

	// Simulate first few turns (no context injection)
	for i := 1; i <= 3; i++ {
		ctx.IncrementTurn()
		fmt.Printf("Turn %d (no injection yet):\n", i)
		fmt.Println("Prompt:", ctx.InjectContextIntoPrompt(prompt))
		fmt.Println()
	}

	// After threshold, inject context usage
	ctx.IncrementTurn()
	fmt.Printf("Turn %d (injecting context):\n", ctx.turnCount)
	fmt.Println("Prompt:", ctx.InjectContextIntoPrompt(prompt))
	fmt.Println()

	// Show volatile content that might be in stable prefix
	fmt.Println("Volatile content that might be injected into stable prefix:")
	fmt.Println("  time: 14:08")
	fmt.Println("  session_id: ses_123456")
	fmt.Println("  timestamp: 2026-09-28T14:08:00Z07:00")
}
