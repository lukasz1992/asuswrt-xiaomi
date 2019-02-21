KBUILD_CFLAGS += -I$(srctree)

# MTK APSoC Platform Support
snd-soc-mt76xx-pcm-objs := mt76xx_pcm.o
snd-soc-mt76xx-i2s-objs := mt76xx_i2s.o

obj-$(CONFIG_SND_MT76XX_PCM) += snd-soc-mt76xx-pcm.o
obj-$(CONFIG_SND_MT76XX_I2S) += snd-soc-mt76xx-i2s.o

# MTK APSoC Machine Support
snd-soc-mt76xx-machine-objs := mt76xx_machine.o

obj-$(CONFIG_SND_MT76XX_SOC) += snd-soc-mt76xx-machine.o



