namespace leopph
{
    public struct Extent2D<T>
    {
        public T width;
        public T height;

        public Extent2D(T width, T height) => (this.width, this.height) = (width, height);
    }
}
