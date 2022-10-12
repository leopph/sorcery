namespace leopph
{
    public struct Extent2D
    {
        public uint width;
        public uint height;

        public Extent2D(uint width, uint height) => (this.width, this.height) = (width, height);
    }
}
