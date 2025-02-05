package imp.ast.typing.data.value;

import com.microsoft.z3.Context;
import com.microsoft.z3.Sort;
import imp.ast.ASTVisitor;
import imp.ast.expression.TypeVisitor;

public class IntType extends PrimitiveType {
    private static final IntType instance = new IntType();

    private IntType() {
    }

    public static IntType getInstance() {
        return instance;
    }

    @Override
    public boolean equals(Object obj) {
        return obj instanceof IntType;
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
