package imp.ast.typing;

import com.microsoft.z3.Context;
import com.microsoft.z3.Sort;
import imp.ast.ASTVisitor;
import imp.ast.expression.TypeVisitor;
import imp.ast.typing.data.DataType;

public final class VoidType extends DataType {
    private static final VoidType instance = new VoidType();

    private VoidType() {
    }

    public static VoidType getInstance() {
        return instance;
    }


    @Override
    public boolean equals(Object obj) {
        return obj instanceof VoidType;
    }

    @Override
    public int hashCode() {
        return getClass().getName().hashCode();
    }

    @Override
    public void accept(TypeVisitor v) {
        v.visit(this);
    }
}
