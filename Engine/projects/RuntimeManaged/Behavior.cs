using System.Runtime.CompilerServices;

namespace leopph
{
    public class Behavior : NativeWrapper
    {
        private Entity? _entity;


        public Entity Entity
        {
            get
            {
                if (_entity == null)
                {
                    _entity = new Entity(InternalGetEntityId(_id));
                }
                return _entity;

            }
        }


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static ulong InternalGetEntityId(ulong behaviorId);
    }
}
